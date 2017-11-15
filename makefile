assembler : main.o assembler.o usedMethods.o
	gcc -g -ansi -Wall main.o assembler.o usedMethods.o -o assembler
main.o : main.c assembler.h defines.h
	gcc -c -ansi -Wall main.c -o main.o 
assembler.o : assembler.c assembler.h usedMethods.h defines.h structs.h
	gcc -c -ansi -Wall assembler.c -o assembler.o
usedMethods.o : usedMethods.c usedMethods.h defines.h structs.h
	gcc -c -ansi -Wall usedMethods.c -o usedMethods.o
