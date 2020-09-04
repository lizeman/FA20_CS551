# DEBUG=-g
test: my_malloc.o my_test.o
	gcc $(DEBUG) -o test my_malloc.o my_test.o

my_test.o: my_malloc.h my_test.c
	gcc $(DEBUG) -c my_test.c

my_malloc.o: my_malloc.h my_malloc.c
	gcc $(DEBUG) -c my_malloc.c

clean: 
	rm test my_test.o my_malloc.o
