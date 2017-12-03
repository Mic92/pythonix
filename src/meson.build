src = [
  'nix-to-python.cc',
  'python-to-nix.cc',
  'eval.cc',
  'python-module.cc',
]

add_global_arguments(['-std=c++17', '-fvisibility=hidden'], language : 'cpp')

python_mod.extension_module('nix', src,
  dependencies : [python_dep, nix_expr_dep],
  install: true,
  install_dir: python_mod.sysconfig_path('platlib')
)
