
all: bin
	cc -Wall -o bin/trip src/trip.c
	./bin/trip -s doc/hello.3p 
	./bin/trip -c doc/spec.3p > bin/s.c	
	cc -Wall -g -o bin/spec bin/s.c 
	./bin/spec
	#gdb --args ./bin/trip -s doc/spec.3p HELLO World
	./bin/trip -s doc/html_sample.3p 
	./bin/trip -s doc/spec.3p HELLO World
	./bin/trip -c doc/edit.3p > bin/e.c	
	cc -Wall  -g -o bin/edit bin/e.c 

bin:
	mkdir -p bin

clean:
	rm -f ./bin/*

