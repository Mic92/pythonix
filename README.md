# Python3 bindings for nix

[![Build Status](https://travis-ci.org/Mic92/pythonix.svg?branch=master)](https://travis-ci.org/Mic92/pythonix)

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
>>> nix.eval('(1 + a)', vars=dict(a=2))
3
>>> nix.eval("builtins.currentSystem")
'x86_64-linux'
>>> nix.eval("with import <nixpkgs> {}; lib.nixpkgsVersion")
'18.03.git.15c484a965a'
```
