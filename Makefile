
all: bin bin/trip
	./bin/trip -c samples/vm3o3.3p > bin/vm.c	
	cc -Wall -g -o bin/vm bin/vm.c 
	./bin/vm samples/hello.3px

bin/trip: src/main.c src/trip.c samples/spec.3p
	cc -Wall -o bin/trip src/main.c
	./bin/trip -s samples/hello.3p 
	./bin/trip -c samples/spec.3p > bin/s.c	
	cc -Wall -g -o bin/spec bin/s.c 
	./bin/spec
	#gdb --args ./bin/trip -s samples/spec.3p HELLO World
	./bin/trip -s samples/html_sample.3p 
	./bin/trip -s samples/spec.3p HELLO World
	./bin/trip -c samples/edit.3p > bin/e.c	
	cc -Wall  -g -o bin/edit bin/e.c 

bin:
	mkdir -p bin

clean:
	rm -f ./bin/*

