{
  description = "LLVM";

  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    #system = "x86_64-linux";
    system = "aarch64-darwin";
    pkgs = import nixpkgs {
      inherit system;
    };
  in {
    devShells.${system}.default = pkgs.mkShell {
      inputsFrom = [
        pkgs.clang_14.cc
      ];
      packages = with pkgs; [
        ninja
      ];
    };
  };
}
