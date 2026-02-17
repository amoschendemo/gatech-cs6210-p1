all: compile

compile:
	gcc -g -Wall memory_coordinator.c virt_query.c coordinator.c -o memory_coordinator -lvirt

clean:
	rm -f memory_coordinator

test:
	gcc -g -Wall test_coordinator.c coordinator.c -o test_coordinator