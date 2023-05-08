.PHONY: all
all: task stdinExample

task:	codec.h basic_main.c
	gcc basic_main.c -L. -l Codec -o encoder

stdinExample:	thread_pool.h thread_pool.c
		gcc thread_pool.c -L. -l Codec -o tester -pthread

.PHONY: clean
clean:
	-rm encoder tester 2>/dev/null
