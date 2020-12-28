{
  description = "Eval nix code from python";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs";
  outputs = { self, nixpkgs }: {
    packages.x86_64-linux.pythonix = nixpkgs.legacyPackages.x86_64-linux.pkgs.callPackage ./. {};
    defaultPackage.x86_64-linux = self.packages.x86_64-linux.pythonix;
  };
}
