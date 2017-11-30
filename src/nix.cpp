#include <nix/config.h>
#include <eval.hh>
#include <store-api.hh>
#include <Python.h>

static PyObject *NixError;

struct PyObjectDeleter {
    void operator () (PyObject* const obj) {
        Py_DECREF (obj);
    }
};

typedef std::unique_ptr<PyObject, PyObjectDeleter> PyObjPtr;

static PyObject * nixValueToPythonObject(nix::EvalState &state, nix::Value &v, nix::PathSet &context) {
    switch (v.type) {
        case nix::tInt:
            return PyLong_FromLong(v.integer);

        case nix::tBool:
            if (v.boolean) {
              Py_RETURN_TRUE;
            } else {
              Py_RETURN_FALSE;
            }
        case nix::tString:
            copyContext(v, context);
            return PyUnicode_FromString(v.string.s);

        case nix::tPath:
            return PyUnicode_FromString(state.copyPathToStore(context, v.path).c_str());

        case nix::tNull:
            Py_RETURN_NONE;

        case nix::tAttrs: {
            auto i = v.attrs->find(state.sOutPath);
            if (i == v.attrs->end()) {
                PyObjPtr dict(PyDict_New());
                if (!dict) {
                    return (PyObject *) nullptr;
                }

                nix::StringSet names;

                for (auto & j : *v.attrs) {
                    names.insert(j.name);
                }
                for (auto & j : names) {
                    nix::Attr & a(*v.attrs->find(state.symbols.create(j)));

                    auto value = nixValueToPythonObject(state, *a.value, context);
                    if (value == nullptr) {
                        return nullptr;
                    }
                    PyDict_SetItemString(dict.get(), j.c_str(), value);
                }
                return dict.release();
            } else {
                return nixValueToPythonObject(state, *i->value, context);
            }
        }

        case nix::tList1: case nix::tList2: case nix::tListN: {
            PyObjPtr list(PyList_New(v.listSize()));
            if (!list) {
                return (PyObject *) nullptr;
            }

            for (unsigned int n = 0; n < v.listSize(); ++n) {
                auto value = nixValueToPythonObject(state, *v.listElems()[n], context);
                if (value == nullptr) {
                    return nullptr;
                }
                PyList_SET_ITEM(list.get(), n, value);
            }
            return list.release();
        }

        case nix::tExternal:
            return PyUnicode_FromString("unevaluated");

        case nix::tFloat:
            return PyFloat_FromDouble(v.fpoint);

        default:
            PyErr_Format(NixError, "cannot convert nix type '%s' to a python object", showType(v));
            return nullptr;
    }
}

const static int envSize = 32768;

static PyObject* eval(const char* expression) {
    nix::Strings storePath;
    nix::EvalState state(storePath, nix::openStore());

    auto env = &state.allocEnv(envSize);
    env->up = &state.baseEnv;

    nix::StaticEnv staticEnv(false, &state.staticBaseEnv);
    staticEnv.vars.clear();

    auto e = state.parseExprFromString(expression, ".", staticEnv);
    nix::Value v;
    e->eval(state, *env, v);

    state.forceValueDeep(v);
    nix::PathSet context;

    return nixValueToPythonObject(state, v, context);
}


static PyObject * nixEval(PyObject *self, PyObject *args) {
    const char *expression;

    if (!PyArg_ParseTuple(args, "s", &expression)) {
        return NULL;
    }

    try {
      return eval(expression);
    } catch (nix::Error & e) {
      PyErr_Format(NixError, "%s", e.what());

      return nullptr;
    } catch (...) {
      std::exception_ptr p = std::current_exception();
      auto name = p ? p.__cxa_exception_type()->name() : "null";

      PyErr_Format(NixError, "unexpected C++ exception: '%s'", name);

      return nullptr;
    }
}

static PyMethodDef NixMethods[] = {
    {"eval", nixEval, METH_VARARGS, "Eval nix expression"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef nixmodule = {
    PyModuleDef_HEAD_INIT,
    "nix",
    "Nix expression bindings",
    -1, /* size of per-interpreter state of the module,
           or -1 if the module keeps state in global variables. */
    NixMethods
};

extern "C" {
    PyMODINIT_FUNC PyInit_nix(void) {
        nix::initGC();

        PyObjPtr m (PyModule_Create(&nixmodule));

        if (!m) {
            return nullptr;
        }

        NixError = PyErr_NewExceptionWithDoc(
            "nix.NixError", /* char *name */
            "Base exception class for the nix module.", /* char *doc */
            NULL, /* PyObject *base */
            NULL /* PyObject *dict */
        );

        if (!NixError) {
            return nullptr;
        }

        if (PyModule_AddObject(m.get(), "NixRef", NixError) == -1) {
          return nullptr;
        }

        return m.release();
    }
}
