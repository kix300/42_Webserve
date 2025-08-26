let
  pkgs = import <nixpkgs> {};
in pkgs.mkShell {
  packages = [
	pkgs.siege
    (pkgs.python3.withPackages (python-pkgs: [
      python-pkgs.pandas
      python-pkgs.requests
    ]))
  ];
}
