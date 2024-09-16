comp-exec:
	gcc *.c -o ./bin/main.out -lm -lz;valgrind ./bin/main.out -s;
comp:
	gcc *.c -o ./bin/main.out -lm -lz; 
exec-dev:
	valgrind ./bin/main.out -s;
