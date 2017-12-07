with import <nixpkgs> {};

(callPackage ./default.nix {}).overrideAttrs(old: {
  buildInputs = old.buildInputs ++ [
    clang-tools
    bashInteractive
  ];
})
