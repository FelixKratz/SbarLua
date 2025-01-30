{
  pkgs ? import <nixpkgs> { },
}:

{
  sbarlua = pkgs.callPackage ./pkgs/sbarlua.nix { };
}
