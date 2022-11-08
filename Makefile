test:
	echo "Hello World"

docker-test:
	docker run --rm -v `pwd`:/ucc -w /ucc compilerbook make test

docker-run:
	docker run --rm -it -v `pwd`:/ucc -w /ucc compilerbook
