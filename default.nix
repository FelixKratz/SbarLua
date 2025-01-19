{
  pkgs ? import <nixpkgs> { },
}:

{
  sbar-lua = pkgs.callPackage ./pkgs/sbar-lua { };
}
