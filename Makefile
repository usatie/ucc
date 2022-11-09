CFLAGS=-std=c11 -g -static -I include

ucc: src/ucc.c
	cc $(CFLAGS) -o ucc src/ucc.c

test: ucc
	bash -x test.sh

clean:
	rm -f ucc *.o *~ tmp*

docker-test:
	docker run --rm -v `pwd`:/ucc -w /ucc compilerbook make test

run:
	docker run --rm -it -v `pwd`:/ucc -w /ucc compilerbook

.PHONY: test clean docker-test run
