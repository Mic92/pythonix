with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "pythonix";
  buildInputs = [
    bashInteractive
    nixUnstable
    meson
    pkgconfig
    ninja
    gcc7
  ];
  src = ./.;
}
