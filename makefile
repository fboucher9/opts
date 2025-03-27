test.exe : opt_main.c opt_decoder.c opt_decoder.h makefile
	gcc -g -O0 -o test.exe -Wall -Wextra -Werror -ansi -pedantic -I. opt_main.c opt_decoder.c
