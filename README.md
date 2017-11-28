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
python3> import nix
python3> nix.eval('(1 + 1)')
2
```
