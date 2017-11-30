# Python3 bindings for nix

status: works for me

## Build from source

```console
$ meson . build
$ ninja -C build
```

or:

```console
$ nix-build
```

## Usage

```console
$ cd build/src
$ python3
>>> import nix
>>> nix.eval('(1 + 1)')
2
>>> nix.eval("builtins.currentSystem")
'x86_64-linux'
>>> nix.eval("with import <nixpkgs> {}; lib.nixpkgsVersion")
'18.03.git.15c484a965a'
```
