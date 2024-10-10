{
  description = "LLVM";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }: let
    system = "aarch64-linux";
    pkgs = import nixpkgs {
      inherit system;
    };
  in {
    devShells.${system}.default = pkgs.mkShell {
      inputsFrom = [
        pkgs.clang_14.cc
#        pkgs.clang_14.lldb
      ];
      packages = with pkgs; [
        cmake
        ninja
        clang_14
#        python3
#        libxml2
      ]
      ;
    };
  };
}
