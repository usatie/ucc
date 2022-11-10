CFLAGS=-std=c11 -g -static -I include

ucc: src/ucc.o src/parse.o src/codegen.o
	cc $(CFLAGS) -o ucc $^

test: ucc
	bash test.sh

debug-test: ucc
	bash -x test.sh

clean:
	rm -f ucc *.o *~ tmp*

docker-test:
	docker run --rm -v `pwd`:/ucc -w /ucc compilerbook make test

run:
	docker run --rm -it --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" -v `pwd`:/ucc -w /ucc compilerbook /bin/bash

.PHONY: test clean docker-test run
