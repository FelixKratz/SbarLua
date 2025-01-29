{
  gcc,
  readline,
  darwin,
  lib,
  stdenv,
  fetchFromGitHub,
}:
stdenv.mkDerivation {
  pname = "sbar-lua";
  version = "unstable-2025-01-20";

  src = fetchFromGitHub {
    owner = "FelixKratz";
    repo = "SbarLua";
    rev = "437bd2031da38ccda75827cb7548e7baa4aa9978";
    hash = "sha256-F0UfNxHM389GhiPQ6/GFbeKQq5EvpiqQdvyf7ygzkPg=";
  };

  buildInputs = [
    gcc
    readline
    darwin.apple_sdk.frameworks.CoreFoundation
  ];

  buildPhase = ''
    make bin/sketchybar.so
  '';

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
