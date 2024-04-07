{
  description = "LLVM";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }: let
    #system = "x86_64-linux";
    system = "aarch64-darwin";
    pkgs = import nixpkgs {
      inherit system;
    };
    frameworks = pkgs.darwin.apple_sdk.frameworks;
    #pkgsCross = import nixpkgs {
    #  inherit system;
    #  crossSystem = {
    #    config = "aarch64-unknown-linux-gnu";
    #  };
    #};
  in {
    devShells.${system}.default = pkgs.mkShell {
#      inputsFrom = [
#        pkgs.clang_14.cc
#        pkgs.clang_14.lldb
#      ];
      packages = with pkgs; [
        cmake
        ninja
#        python3
#        libxml2
        frameworks.CoreServices
      ]
#      ++ lib.optional stdenv.hostPlatform.isDarwin fixDarwinDylibNames
      ;
    };
  };
}
