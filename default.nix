{
  lib,
  stdenv,
  curl,
}:
stdenv.mkDerivation {
  pname = "hxxp";
  version = "0.0.1";

  src = ./.;

  buildInputs = [ curl ];

  makeFlags = [ "OUT=$(out)" ];

  meta = with lib; {
    description = "HXXP — eXtended HTTP runner";
    homepage = "https://github.com/hissssst/hxxp";
    mainProgram = "hxxp";
    platforms = platforms.unix;
    license = licenses.bsd2;
  };
}
