#pragma once

#include <Python.h>
#include <eval.hh>
#include <nix/config.h>
#include <optional>

namespace pythonnix {

nix::Value *pythonToNixValue(nix::EvalState &state, PyObject *obj);

std::optional<nix::StaticEnv> pythonToNixEnv(nix::EvalState &state,
                                             PyObject *vars, nix::Env **env);
} // namespace pythonnix
