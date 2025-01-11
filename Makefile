
all: bin
	cc -Wall -o bin/trip src/trip.c; 
	./bin/trip doc/hello.3p > bin/h.c; 
	cc -o bin/hello bin/h.c ; 
	./bin/hello
	./bin/trip doc/spec.3p	

bin:
	mkdir -p bin

clean:
	rm -f ./bin/*

