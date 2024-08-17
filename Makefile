compile:
	gcc *.c -o ./bin/main.out -lm; 
	valgrind ./bin/main.out -s;
