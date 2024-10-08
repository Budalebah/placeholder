{ lib,
  stdenv,
  cmake,
  cmake_modules,
  pkg-config,
  boost,
  clang,
  clang-tools,
  gtk4,
  gtkmm4,
  glibmm,
  pcre2,
  glib,
  gdb,
  lldb,
  ninja,
  pango,
  pangomm,
  crypto3,
  }:
let
  inherit (lib) optional;
in stdenv.mkDerivation {
  name = "debug-tools";

  src = lib.sourceByRegex ./. [ ".*" ];

  nativeBuildInputs = [ cmake ninja pkg-config ] ++
                       (lib.optional (!stdenv.isDarwin) gdb) ++
                       (lib.optional (stdenv.isDarwin) lldb);

  propagatedBuildInputs = [ boost crypto3 gtk4 gtkmm4 glibmm pcre2 glib pango pangomm ];

  buildInputs = [ cmake_modules ];

  cmakeFlags =
    [
      "-DCMAKE_BUILD_TYPE=Release"
      "-G Ninja"
    ];

  shellHook = ''
    PS1="\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ "
    echo "Welcome to debug-tools development environment!"
  '';
}