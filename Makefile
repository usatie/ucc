NAME=ucc
CFLAGS=-std=gnu11 -g -static -I include
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

$(OBJS): include/ucc.h

test: $(NAME)
	./test.sh

clean:
	rm -f $(NAME) **/*.o *~ tmp*

re: clean $(NAME)

docker-test:
	docker run --rm -v `pwd`:/$(NAME) -w /$(NAME) compilerbook make test

run:
	docker run --rm -it --cap-add=SYS_PTRACE --security-opt="seccomp=unconfined" -v `pwd`:/$(NAME) -w /$(NAME) compilerbook /bin/bash

.PHONY: test clean re docker-test run
