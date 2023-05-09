.PHONY: all
all: task thread_pool

task:	codec.h basic_main.c
	gcc basic_main.c -L. -l Codec -o encoder

thread_pool:	thread_pool.h thread_pool.c
		gcc -g thread_pool.c ./libCodec.so -o tester -pthread

.PHONY: clean
clean:
	-rm encoder tester 2>/dev/null
