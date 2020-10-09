# Very unsophisticated Makefile, 
# DEBUG=-g

myar: pipesort.o 
	gcc $(DEBUG) -o pipesort pipesort.o

myar.o: piposort.c
	gcc $(DEBUG) -c pipesort.c

clean: 
	rm pipesort pipesort.o
