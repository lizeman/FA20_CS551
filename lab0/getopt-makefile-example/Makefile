# Very unsophisticated Makefile
#DEBUG=-g

welcome: welcome.o printit.o
	gcc $(DEBUG) -o welcome welcome.o printit.o

welcome.o: name.h welcome.c
	gcc $(DEBUG) -c welcome.c 

printit.o: name.h printit.c
	gcc $(DEBUG) -c printit.c

clean: 
	rm welcome welcome.o printit.o
