# Very unsophisticated Makefile, 
# DEBUG=-g
DEBUG=-Wno-format-truncation

myar: myar.o 
	gcc $(DEBUG) -o myar myar.o 

myar.o: myar.c
	gcc $(DEBUG) -c myar.c

clean: 
	rm myar myar.o
