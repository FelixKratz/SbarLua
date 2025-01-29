{
  gcc,
  readline,
  darwin,
  lib,
  lua54Packages,
}:

lua54Packages.buildLuaPackage {
  name = "sbarlua";
  pname = "sbarlua";
  version = "unstable-2025-01-20";

  src = ../.;

  buildInputs = [
    gcc
    readline
    darwin.apple_sdk.frameworks.CoreFoundation
  ];

  installPhase = ''
    mkdir -p $out/lib
    mv bin/sketchybar.so $out/lib/sketchybar.so
  '';

  meta = {
    description = "A Lua API for SketchyBar";
    homepage = "git@github.com:FelixKratz/SbarLua.git";
    license = lib.licenses.gpl3Only;
    maintainers = with lib.maintainers; [
      lalit64
      amusingimpala75
    ];
    mainProgram = "sbarlua";
    platforms = lib.platforms.darwin;
  };
}
