CFLAGS = -O3

va: va.c

test: va
	@sh ./run_tests.sh

clean:
	rm -f va
