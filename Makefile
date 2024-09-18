comp-exec:
	gcc *.c -o ./bin/main.out -lm -lz;valgrind -s ./bin/main.out $(file);
comp:
	gcc *.c -o ./bin/main.out -lm -lz; 
exec-dev:
	valgrind -s ./bin/main.out $(file);
