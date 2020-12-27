{
  pkgs ? (import <nixpkgs> {}).pkgs
}:

with pkgs;

let
  filterMesonBuild = dir: builtins.filterSource
    (path: type: type != "directory" || baseNameOf path != "build") dir;
  python = python3;
in python.pkgs.buildPythonPackage rec {
  name = "pythonix";
  format = "other";

  nativeBuildInputs = [
    ninja
    (meson.override { python3 = python3; })
    pkgconfig
    gcc_latest
  ];

  checkPhase = ''
    ninja test
  '';

  buildInputs = [ nix boost ];
  src = filterMesonBuild ./.;
}
