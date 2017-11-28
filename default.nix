with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "env";
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
