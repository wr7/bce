{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs.buildPackages; [
    meson
    ninja
    gcc
    clang-tools # Can be removed but is necessary for clangd
  ];
}
