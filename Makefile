compile:
	gcc *.c -o ./bin/main.out -lm -lz; 
	valgrind ./bin/main.out -s;
