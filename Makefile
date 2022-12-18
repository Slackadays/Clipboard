install-linux:
	@printf "=> Clipboard Makefile v1.0.0\n"
	@printf "=> Made by gentoo-btw\n"
	@test -f /usr/bin/cmake && printf "=> Found cmake at /usr/bin/cmake\n"
	@test -f /usr/bin/cmake || printf "=> cmake not found. please install cmake.\n"
	@read -p "=> You are about to install Clipboard. Are you sure you want to do this?. If you do. Press ENTER to install Clipboard."
	@printf "=> cmake Clipboard/src"
	@cmake Clipboard/src
	@printf "=> cmake --build .\n"
	@cmake --build .
	@printf "=> sudo cmake --install .\n"
	@cmake --install .
	@printf "=> Thank you for installing Clipboard.\n"

install-win:
	@printf "=> Clipboard Makefile v1.0.0\n"
	@printf "=> Made by gentoo-btw\n"
	@read -p "=> You are about to install Clipboard. Are you sure you want to do this?. If you do. Press ENTER to install Clipboard."
	@printf "=> cmake Clipboard/src\n"
	@cmake Clipboard/src
	@printf "=> cmake --build .\n"
	@cmake --build .
	@printf "=> cmake --install .\n"
	@cmake --install .
	@printf "=> Thank you for installing Clipboard.\n"

install-bsd:
	@printf "=> Clipboard Makefile v1.0.0\n"
	@printf "=> Made by gentoo-btw\n"
	@test -f /usr/bin/cmake && printf "=> Found cmake at /usr/bin/cmake\n"
	@test -f /usr/bin/cmake || printf "=> cmake not found. please install cmake.\n"
	@read -p "=> You are about to install Clipboard. Are you sure you want to do this?. If you do. Press ENTER to install Clipboard."
	@printf "=> cmake Clipboard/src\n"
	@cmake Clipboard/src
	@printf "=> cmake --build .\n"
	@cmake --build .
	@printf "=> doas cmake --install .\n"
	@doas cmake --install .
	@printf "=> Thank you for installing Clipboard.\n"
