{
  description = "Cut, copy, and paste anything, anywhere, all from the terminal! Save time and effort the easy way.";

  outputs = { self, nixpkgs }: {
    packages.x86_64-linux.default =
      let pkgs = import nixpkgs {
            system = "x86_64-linux";
          };
      in pkgs.stdenv.mkDerivation {
        pname = "slackadays-clipboard";
        version = "0.3.2";
        src = pkgs.fetchgit {
          url = "https://github.com/Slackadays/Clipboard";
          rev = "0.3.2";
          sha256 = "xdogl2WDuQXeLFuBY1u7PSpaoVI9HKScOdxHZ3+whIg=";
        };

        nativeBuildInputs = with pkgs; [
          cmake
        ];
      };
  };
}
