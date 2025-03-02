
all: bin
	cc -Wall -o bin/trip src/trip.c; 
	./bin/trip -c doc/hello.3p > bin/h.c; 
	cc -Wall -o bin/hello bin/h.c 
	./bin/hello
	./bin/trip -c doc/spec.3p > bin/s.c	
	cc -Wall -g -o bin/spec bin/s.c ;
	./bin/spec
	#gdb --args ./bin/trip -s doc/spec.3p HELLO World
	./bin/trip -s doc/spec.3p HELLO World
	./bin/trip -s doc/html_sample.3p 

bin:
	mkdir -p bin

clean:
	rm -f ./bin/*

