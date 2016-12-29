.PHONY: all test clean

all:  out/test

test: out/test
	./out/test -v

out/test: clist_test.c cyclist.h debug.h
	gcc -o out/test clist_test.c

clean:
	git clean -fdx out
