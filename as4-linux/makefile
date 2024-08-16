all:
	arm-linux-gnueabihf-gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -pthread as4-morsecode.c function.c LEDmatrix.c morsecode.c keyboard.c pru.c -o as4-morsecode
	cp as4-morsecode $(HOME)/cmpt433/public/myApps/