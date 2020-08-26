# Very unsophisticated Makefile
#DEBUG=-g

ppmcvt: ppmcvt.o
	gcc $(DEBUG) -o ppmcvt ppmcvt.o

ppmcvt.o: ppmcvt.h ppmcvt.c
	gcc $(DEBUG) -c ppmcvt.c

clean: 
	rm ppmcvt ppmcvt.o
