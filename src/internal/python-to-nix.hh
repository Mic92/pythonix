#pragma once

#include <nix/config.h>

#include <Python.h>
#include <eval.hh>

namespace pythonnix {

nix::Value *pythonToNixValue(nix::EvalState &state, PyObject *obj);

std::optional<nix::StaticEnv> pythonToNixEnv(nix::EvalState &state,
                                             PyObject *vars, nix::Env **env);
}
