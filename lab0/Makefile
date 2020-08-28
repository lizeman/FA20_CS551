# Very unsophisticated Makefile
#DEBUG=-g

ppmcvt: pbm.o cvtlib.o ppmcvt.o
	gcc $(DEBUG) -o ppmcvt pbm.o cvtlib.o ppmcvt.o

ppmcvt.o: pbm.h ppmcvt.h ppmcvt.c
	gcc $(DEBUG) -c ppmcvt.c

cvtlib.o: pbm.h ppmcvt.h cvtlib.c
	gcc $(DEBUG) -c cvtlib.c

pbm.o: pbm.h pbm.c
	gcc $(DEBUG) -c pbm.c

clean: 
	rm ppmcvt ppmcvt.o pbm.o
