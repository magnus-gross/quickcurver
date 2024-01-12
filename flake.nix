{
	description = "Modern Qt/C++ implementation of Achtung die Kurve with online multiplayer";
	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
	};

	outputs = { self, nixpkgs }: {
		packages = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed (system:
		let
			pkgs = nixpkgs.legacyPackages.${system};
			quartz = pkgs.fetchFromGitHub {
				owner = "vimpostor";
				repo = "quartz";
				rev = builtins.head (builtins.match ".*FetchContent_Declare\\(.*GIT_TAG ([[:alnum:]\\.]+).*" (builtins.readFile ./CMakeLists.txt));
				hash = "sha256-sbOoYO87NRF3J3FOnCAM40BtXPtdfyYPPfiNgK3q56o=";
			};
		in {
			default = pkgs.stdenv.mkDerivation {
				pname = "quickcurver";
				version = "0.1";

				src = ./.;

				nativeBuildInputs = with pkgs; [
					cmake
					pkg-config
					qt6.wrapQtAppsHook
					imagemagick
				];
				buildInputs = with pkgs; [
					qt6.qtbase
					qt6.qtdeclarative
					qt6.qtsvg
				];
				cmakeFlags = [("-DFETCHCONTENT_SOURCE_DIR_QUARTZ=" + quartz)];
				postBuild = "make linux-desktop-integration";
			 };
		});
	};
}
