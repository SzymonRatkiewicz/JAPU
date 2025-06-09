comp-exec:
	gcc *.c -o ./bin/japu -lm -lz;valgrind -s ./bin/japu $(file);
comp:
	gcc *.c -o ./bin/japu -lm -lz; 
comp-stat:
	gcc *.c -static -o ./bin/japu -lm -lz; 
exec-dev:
	valgrind -s ./bin/japu $(file);
build-docker:
	mkdir -p ./bin/docker &&\
	docker build -t japu-builder . && \
	docker run --rm -v $(shell pwd)/bin/docker:/output japu-builder cp /app/bin/japu /output
