#include "name.h"
#include <getopt.h>
#include "string.h"

// -m print mask greeting, -n supply first-name

int main (char argc, char *argv[]) {
	int opt, mask=0; char *name = NULL;

	while ((opt = getopt(argc, argv, "mn:")) != -1) {
		switch (opt) {
		case 'n': 
			name= optarg;
			break;
		case 'm': mask=1; 	
			break;
		}
	}

	if (name == NULL) name = DEFAULT_NAME;
	printit(mask,name);
}


