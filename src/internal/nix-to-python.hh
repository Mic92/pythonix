#pragma once

#include <nix/config.h>

#include <Python.h>
#include <eval.hh>

namespace pythonnix {

PyObject *nixToPythonObject(nix::EvalState &state, nix::Value &v,
                            nix::PathSet &context);
}
