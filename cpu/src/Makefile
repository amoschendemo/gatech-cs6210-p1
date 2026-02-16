all: compile

compile:
	gcc -g -Wall vcpu_scheduler.c mcmf.c graph.c scheduler.c virt_query.c -o vcpu_scheduler -lvirt -lm

clean:
	rm -f vcpu_scheduler

test:
	gcc -Wall -Wextra -O2 -o test_mcmf test_mcmf.c mcmf.c graph.c -lm