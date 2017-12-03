#include "internal/nix-to-python.hh"
#include "internal/errors.hh"
#include "internal/ptr.hh"

namespace pythonnix {

PyObject *nixToPythonObject(nix::EvalState &state, nix::Value &v,
                            nix::PathSet &context) {
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
        return (PyObject *)nullptr;
      }

      nix::StringSet names;

      for (auto &j : *v.attrs) {
        names.insert(j.name);
      }
      for (auto &j : names) {
        nix::Attr &a(*v.attrs->find(state.symbols.create(j)));

        auto value = nixToPythonObject(state, *a.value, context);
        if (!value) {
          return nullptr;
        }
        PyDict_SetItemString(dict.get(), j.c_str(), value);
      }
      return dict.release();
    } else {
      return nixToPythonObject(state, *i->value, context);
    }
  }

  case nix::tList1:
  case nix::tList2:
  case nix::tListN: {
    PyObjPtr list(PyList_New(v.listSize()));
    if (!list) {
      return (PyObject *)nullptr;
    }

    for (unsigned int n = 0; n < v.listSize(); ++n) {
      auto value = nixToPythonObject(state, *v.listElems()[n], context);
      if (!value) {
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
    PyErr_Format(NixError, "cannot convert nix type '%s' to a python object",
                 showType(v));
    return nullptr;
  }
}
}
