#include "name.h"
#include <stdio.h>

void printit(int mask, char * name) {
	char *phrase;
	phrase = mask ? MASK : DIST; 
	printf ("Welcome %s %s\n",name,phrase);
}
