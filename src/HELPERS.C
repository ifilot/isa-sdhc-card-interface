#include "helpers.h"

void print_block(unsigned char buf[]) {
    int i,j;
    unsigned char *ptr = buf;

    for(i=0; i<16; ++i) {
	for(j=0; j<16; ++j) {
	    printf("%02X ", *(ptr++));
	}
	printf("\n");
    }
    printf("Press any key to continue to next 256 bytes.");
    getch();
    printf("\r");
    for(i=0; i<16; ++i) {
	for(j=0; j<16; ++j) {
	    printf("%02X ", *(ptr++));
	}
	printf("\n");
    }
}