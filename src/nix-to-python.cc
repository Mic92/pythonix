#include <Python.h>

#include "internal/nix-to-python.hh"
#include "internal/errors.hh"
#include "internal/ptr.hh"

#include <iostream>

namespace pythonnix {

PyObject *nixPosToPythonObject(nix::Pos &pos) {
  PyObject *filename = PyUnicode_FromString(static_cast<std::string>(pos.file).c_str());
  PyObject *line = PyLong_FromLong(pos.line);
  PyObject *column = PyLong_FromLong(pos.column);
  return PyTuple_Pack(3, filename, line, column);
}

PyObject *nixAttrPathToPythonObject(nix::AttrPath &path) {
  PyObject* type = PyUnicode_FromString("ATTRPATH");

  PyObject* listObj = PyList_New(path.size());
  if (!listObj) { return (PyObject *)nullptr; }

  for (std::size_t n = 0; n < path.size(); ++n) {
    PyObject* expr = nixExprToPythonObject(*(path[n].expr));
    if (!expr) {
      Py_DECREF(listObj);
      return nullptr;
    }
    PyObject* symbol = PyUnicode_FromString(static_cast<std::string>(path[n].symbol).c_str());
    PyObject* tuple = PyTuple_Pack(2, symbol, expr);
    PyList_SET_ITEM(listObj, n, tuple);
  }
  return PyTuple_Pack(2, type, listObj);
}

PyObject *nixExprToPythonObject(nix::Expr &expression) {
  // going to use typeid to determine type
  // I have not found a better method since I cannot
  // change libexpr implementation
  // https://en.cppreference.com/w/cpp/language/typeid
  if(auto e = dynamic_cast<nix::ExprInt*>(&expression)) {
    PyObject *type = PyUnicode_FromString("INT");
    PyObject *value = PyLong_FromLong(e->v.integer);
    return PyTuple_Pack(2, type, value);
  } else if (auto e = dynamic_cast<nix::ExprFloat*>(&expression)) {
    PyObject *type = PyUnicode_FromString("FLOAT");
    PyObject *value = PyFloat_FromDouble(e->v.fpoint);
    return PyTuple_Pack(2, type, value);
  } else if (auto e = dynamic_cast<nix::ExprString*>(&expression)) {
    PyObject *type = PyUnicode_FromString("STRING");
    PyObject *value = PyUnicode_FromString(e->v.string.s);
    return PyTuple_Pack(2, type, value);
  } else if (auto e = dynamic_cast<nix::ExprIndStr*>(&expression)) {
    PyObject *type = PyUnicode_FromString("INDSTR");
    PyObject *value = PyUnicode_FromString(e->s.c_str());
    return PyTuple_Pack(2, type, value);
  } else if (auto e = dynamic_cast<nix::ExprPath*>(&expression)) {
    PyObject *type = PyUnicode_FromString("PATH");
    PyObject *value = PyUnicode_FromString(e->s.c_str());
    return PyTuple_Pack(2, type, value);
  } else if (auto e = dynamic_cast<nix::ExprVar*>(&expression)) {
    PyObject *type = PyUnicode_FromString("PATH");
    PyObject *name = PyUnicode_FromString(static_cast<std::string>(e->name).c_str());
    PyObject *fromWith = e->fromWith ? Py_True : Py_False;
    PyObject *pos = nixPosToPythonObject(e->pos);
    return PyTuple_Pack(4, type, name, fromWith, pos);
  } else if (auto e = dynamic_cast<nix::ExprSelect*>(&expression)) {
    PyObject *type = PyUnicode_FromString("SELECT");
    PyObject *exprFirst = nixExprToPythonObject(*(e->e));
    PyObject *exprSecond = nixExprToPythonObject(*(e->e));
    PyObject *pos = nixPosToPythonObject(e->pos);
    PyObject *attrPath = nixAttrPathToPythonObject(e->attrPath);
    return PyTuple_Pack(5, type, exprFirst, exprSecond, attrPath, pos);
  } else if (auto e = dynamic_cast<nix::ExprOpHasAttr*>(&expression)) {
    PyObject *type = PyUnicode_FromString("OPHASATTR");
    PyObject *value = nixExprToPythonObject(*(e->e));
    PyObject *attrPath = nixAttrPathToPythonObject(e->attrPath);
    return PyTuple_Pack(5, type, value, attrPath);
  } else if (auto e = dynamic_cast<nix::ExprAttrs*>(&expression)) {
    PyObject *dictObj = PyDict_New();
    if (!dictObj) {
       return (PyObject *)nullptr;
    }

    for (auto it=e->attrs.begin(); it!=e->attrs.end(); ++it) {
      auto value = nixExprToPythonObject(*(it->second.e));
      if (!value) {
        Py_DECREF(dictObj);
        return nullptr;
      }
      const char* key = static_cast<std::string>(it->first).c_str();
      PyObject *inherited = it->second.inherited ? Py_True : Py_False;
      PyObject *pos = nixPosToPythonObject(it->second.pos);
      PyObject *displacement = PyLong_FromLong(it->second.displ);
      PyObject *tuple = PyTuple_Pack(3, value, inherited, pos, displacement);
      PyDict_SetItemString(dictObj, key, tuple);
    }

    PyObject *type = PyUnicode_FromString("ATTRS");
    PyObject *recursive = e->recursive ? Py_True : Py_False;
    return PyTuple_Pack(3, type, recursive, dictObj);
  } else if (auto e = dynamic_cast<nix::ExprList*>(&expression)) {
    PyObject* listObj = PyList_New(e->elems.size());
    if (!listObj) { return (PyObject *)nullptr; }

    for (std::size_t n = 0; n < e->elems.size(); ++n) {
      auto value = nixExprToPythonObject(*(e->elems[n]));
      if (!value) {
        Py_DECREF(listObj);
        return nullptr;
      }
      PyList_SET_ITEM(listObj, n, value);
    }

    PyObject *type = PyUnicode_FromString("LIST");
    return PyTuple_Pack(2, type, listObj);
  } else if (auto e = dynamic_cast<nix::ExprLambda*>(&expression)) {
    std::cout << "(LAMBDA, " << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprLet*>(&expression)) {
    std::cout << "LET" << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprWith*>(&expression)) {
    std::cout << "WITH" << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprIf*>(&expression)) {
    std::cout << "IF" << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprAssert*>(&expression)) {
    std::cout << "ASSERT" << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprOpNot*>(&expression)) {
    std::cout << "OPNOT" << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprConcatStrings*>(&expression)) {
    std::cout << "CONCATSTRINGS" << std::endl;
    Py_RETURN_TRUE;
  } else if (auto e = dynamic_cast<nix::ExprPos*>(&expression)) {
    std::cout << "POS" << std::endl;
    Py_RETURN_TRUE;
  } else {
    std::cout << "not sure what it is..." << std::endl;
    Py_RETURN_TRUE;
  }
}

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
                 showType(v).c_str());
    return nullptr;
  }
}
} // namespace pythonnix
