comp-exec:
	gcc *.c -o ./bin/japu -lm -lz;valgrind -s ./bin/japu $(file);
comp:
	gcc *.c -o ./bin/japu -lm -lz; 
exec-dev:
	valgrind -s ./bin/japu $(file);
