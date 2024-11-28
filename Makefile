comp-exec:
	gcc *.c -g -o ./bin/japu -lm -lz;valgrind -s ./bin/japu $(file);
comp:
	gcc *.c -o ./bin/japu -lm -lz; 
exec-dev:
	valgrind -s ./bin/japu $(file);
test:
	gcc *.c -g -o ./bin/japu -lm -lz;valgrind -s ./bin/japu ./resources/gatto.png -w ./out-test.html -d 1000 1000;
testdebug:
	gdb --args ./bin/japu ./resources/gatto.png ./out-test.txt -d 1000 1000;
