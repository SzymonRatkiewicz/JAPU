comp-exec:
	gcc *.c -o ./bin/japu -lm -lz;valgrind -s ./bin/japu $(file) -w ./out.html;
comp:
	gcc *.c -o ./bin/japu -O3 -lm -lz; 
comp-stat:
	gcc *.c -static -o ./bin/japu -lm -lz; 
exec-dev:
	valgrind -s ./bin/japu $(file);
comp-profiler:
	gcc *.c -o ./bin/japu -lm -lz -O0 -g -pg; valgrind --tool="callgrind" --dump-instr=yes --collect-jumps=yes ./bin/japu ./resources/gatto.png;
build-docker:
	mkdir -p ./bin/docker &&\
	docker build -t japu-builder . && \
	docker run --rm -v $(shell pwd)/bin/docker:/output japu-builder cp /app/bin/japu /output
