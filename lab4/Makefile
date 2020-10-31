# Very unsophisticated Makefile, 
# DEBUG=-g

all: compute manage

compute: compute.o 
	gcc $(DEBUG) -o compute compute.o

compute.o: defs.h compute.c
	gcc $(DEBUG) -c compute.c

manage: manage.o
	gcc $(DEBUG) -o manage manage.o

manage.o: defs.h manage.c
	gcc $(DEBUG) -c manage.c

clean: 
	rm compute compute.o manage manage.o
