#include "sd.h"

unsigned char sd_boot() {
    int i, j, ctr;
    unsigned char v, c1, c2;
    unsigned char *ptr;

    clrscr();
    printf("SDCARD interface program\n");
    sd_set_miso_high(BASEPORT);

    ctr = 0;
    v = 0xFF;
    while(v == 0xFF & ctr < 100) {
	sddis(BASEPORT);
	cmdclr(BASEPORT);
	sden(BASEPORT);
	v = cmd00(BASEPORT);	/* put card in idle state */
	ctr++;
    }
    if(ctr == 100) {
	printf("Cannot put card in idle mode. Exiting.");
	sddis(BASEPORT);
	return 1;
    }
    printf("CMD00 response: %02X\n", v);
    v = cmd08(BASEPORT);	/* send cmd08 */
    printf("CMD08 response: %02X ", v);

    /* retrieve remaining bytes of rsp5 */
    for(i=0; i<4; ++i) {
	v = sdrecvf(BASEPORT);
	printf("%02X ", v);
    }
    printf("\n");

    /* TRY ACMD41 */
    v = 0xFF;
    ctr = 0;
    while(v != 0x00 && ctr < 100) {
	v = cmd55(BASEPORT);
	if(v == 0xFF) {
	    break;	/* terminate attempts */
	}
	v = acmd41(BASEPORT);
	ctr++;
    }
    if(ctr == 100) {
	printf("Unable to send ACMD41 after %i attempts.", ctr);
    } else {
	printf("ACMD41 response: %02X (%i)\n", v, ctr);
    }

    /* CMD58 - READ OCR */
    v = cmd58(BASEPORT);
    printf("CMD58 response: %02X ", v);
    for(i=0; i<4; ++i) {
	v = sdrecvf(BASEPORT);
	printf("%02X ", v);
    }
    printf("\n");

    return 0;
}

void sd_set_miso_low(unsigned ioport) {
    inportb(ioport | 0x02); /* ignore return value */
}

void sd_set_miso_high(unsigned ioport) {
    inportb(ioport | 0x03); /* ignore return value */
}

void sd_disable(unsigned ioport) {
    sddis(ioport);
}