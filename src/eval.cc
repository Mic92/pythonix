#include "internal/eval.hh"
#include "internal/errors.hh"
#include "internal/nix-to-python.hh"
#include "internal/python-to-nix.hh"

#include <store-api.hh>

namespace pythonnix {

static PyObject *_eval(const char *expression, PyObject *vars) {
  nix::Strings storePath;
  nix::EvalState state(storePath, nix::openStore());

  nix::Env *env;
  auto staticEnv = pythonToNixEnv(state, vars, &env);
  if (!staticEnv) {
    return nullptr;
  }

  auto e = state.parseExprFromString(expression, ".", *staticEnv);
  nix::Value v;
  e->eval(state, *env, v);

  state.forceValueDeep(v);

  nix::PathSet context;
  return nixToPythonObject(state, v, context);
}

PyObject *eval(PyObject *self, PyObject *args, PyObject *keywds) {
  const char *expression;
  PyObject *vars;

  const char *kwlist[] = {"expression", "vars", nullptr};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "sO!",
                                   const_cast<char **>(kwlist), &expression,
                                   &PyDict_Type, &vars)) {
    return nullptr;
  }

  if (!PyDict_Check(vars)) {
    printf("expect a dict!\n");
    return nullptr;
  }

  try {
    return _eval(expression, vars);
  } catch (nix::Error &e) {
    return PyErr_Format(NixError, "%s", e.what());
  } catch (...) {
    std::exception_ptr p = std::current_exception();
    auto name = p ? p.__cxa_exception_type()->name() : "null";

    return PyErr_Format(NixError, "unexpected C++ exception: '%s'", name);
  }
}
}
