with import <nixpkgs> {};

(callPackage ./release.nix {}).overrideAttrs(old: {
  buildInputs = old.buildInputs ++ [
    clang-tools
    bashInteractive
  ];
})
