#DEBUG=-g
DEBUG=-m32
other_test: my_malloc.o other_test.o
	gcc $(DEBUG) -o other_test my_malloc.o other_test.o
other_test.o: my_malloc.h other_test.c
	gcc $(DEBUG) -c other_test.c

test: my_malloc.o my_test.o
	gcc $(DEBUG) -o test my_malloc.o my_test.o

my_test.o: my_malloc.h my_test.c
	gcc $(DEBUG) -c my_test.c

my_malloc.o: my_malloc.h my_malloc.c
	gcc $(DEBUG) -c my_malloc.c

clean: 
	rm test my_test.o my_malloc.o
