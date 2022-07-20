# Python3 bindings for nix

Maintainer status: unmaintained

I am not really using this project anymore and it would require some porting
changes to newer nix apis.  You can also use the `nix eval --json --expr` to get
json output from nix and use the python json library to parse it again.

An example can be found here:
https://github.com/Mic92/nix-update/blob/3143af043d4324ee4c6a91ff7294c5c34fe459b6/nix_update/eval.py#L77

If you want to adopt the project, drop an email at joerg@thalheim.io or find on matrix in the nixos channel.

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
