
all: bin
	cc -Wall -o bin/trip src/trip.c; 
	./bin/trip doc/hello.3p > bin/h.c; 
	cc -Wall -o bin/hello bin/h.c 
	./bin/hello
	./bin/trip doc/spec.3p > bin/s.c	
	cc -Wall -o bin/spec bin/s.c ;
	./bin/spec

bin:
	mkdir -p bin

clean:
	rm -f ./bin/*

