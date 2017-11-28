nix_sources = ['eval.cpp']

extension = shared_module('nix', 'nix.cpp',
  name_prefix: '',
  name_suffix: 'so',
  dependencies : [python_dep, nix_expr_dep],
  default_options : ['cpp_std=c++14'],
  install: true,
)
