# Python3 bindings for nix

status: WIP

build:

```console
$ meson . build
$ ninja -C build
```

or:

```console
$ nix-build
```


usage:

```console
$ cd build/src
$ python3
>>> import nix
>>> nix.eval('(1 + 1)')
2
>>> nix.eval("builtins.currentSystem")
'x86_64-linux'
>>> with import <nixpkgs> {}; lib.nixpkgsVersion
'18.03.git.15c484a965a'
```
