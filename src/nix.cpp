#include <nix/config.h>
#include <eval.hh>
#include <store-api.hh>
#include <Python.h>
#include <iostream>

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
                PyObject* dict = PyDict_New();
                if (dict == nullptr) {
                    return dict;
                }

                nix::StringSet names;

                for (auto & j : *v.attrs) {
                    names.insert(j.name);
                }
                for (auto & j : names) {
                    nix::Attr & a(*v.attrs->find(state.symbols.create(j)));

                    auto value = nixValueToPythonObject(state, *a.value, context);
                    PyDict_SetItemString(dict, j.c_str(), value);
                }
                return dict;
            } else {
                return nixValueToPythonObject(state, *i->value, context);
            }
        }

        case nix::tList1: case nix::tList2: case nix::tListN: {
            auto list = PyList_New(v.listSize());
            if (list == nullptr) {
                return list;
            }

            for (unsigned int n = 0; n < v.listSize(); ++n) {
                PyList_SET_ITEM(list, n, nixValueToPythonObject(state, *v.listElems()[n], context));
            }
            return list;
        }

        case nix::tExternal:
            return PyUnicode_FromString("unevaluated");

        case nix::tFloat:
            return PyFloat_FromDouble(v.fpoint);

        default:
            // TODO exception
            return PyUnicode_FromFormat("cannot convert %s to JSON", showType(v));
    }
}

static PyObject * eval(PyObject *self, PyObject *args) {
    const char *expression;

    nix::Strings storePath;
    nix::EvalState state(storePath, nix::openStore());
    nix::StaticEnv staticEnv(false, 0);

    if (!PyArg_ParseTuple(args, "s", &expression)) {
        return NULL;
    }

    auto e = state.parseExprFromString(expression, ".", staticEnv);
    nix::Value v;
    state.eval(e, v);

    state.forceValueDeep(v);
    nix::PathSet context;

    return nixValueToPythonObject(state, v, context);
}

static PyMethodDef NixMethods[] = {
    {"eval", eval, METH_VARARGS, "Eval nix expression"},
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
        PyObject *m;
        nix::initGC();

        m = PyModule_Create(&nixmodule);
        if (m == NULL) {
            return NULL;
        }

        return m;
    }
}
