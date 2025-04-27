{ pkgs ? (import <nixpkgs> { } ), ... }:
pkgs.mkShell {
  buildInputs = with pkgs; [
    curl
    gnumake
    gcc
  ];

  nativeBuildInputs = with pkgs; [
    curl
  ];
}
