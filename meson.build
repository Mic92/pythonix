project('python-nix', 'cpp',
  version : '1.0.0',
  license : 'LGPL-2.0',
  default_options : ['cpp_std=c++14']
)

python_mod = import('python3')
python_dep = dependency('python3', required : true)
nix_expr_dep = dependency('nix-expr', required: true)

subdir('src')
