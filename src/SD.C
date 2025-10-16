#include "sd.h"

int sd_boot() {
    int i, j, ctr;
    unsigned char v, c1, c2;
    unsigned char *ptr;
    unsigned char rsp5[5];

    sd_set_miso_high(BASEPORT);

    ctr = 0;
    v = 0xFF;
    while(v != 0x01 & ctr < MAXTRIAL) {
	sddis(BASEPORT);
	cmdclr(BASEPORT);
	sden(BASEPORT);
	v = cmd00(BASEPORT);	/* put card in idle state */
	ctr++;
    }
    if(ctr >= MAXTRIAL) {
	printf("Cannot put card in idle mode.\n");
	printf("Try to reinsert the card.\n");
	sddis(BASEPORT);
	return -1;
    }
    printf("CMD00 response: %02X\n", v);
    v = cmd08(BASEPORT, rsp5);	/* send cmd08 */
    printf("CMD08 response: ");
    for(i=0; i<5; ++i) {
	printf("%02X ", rsp5[i]);
    }
    printf("\n");

    /* TRY ACMD41 */
    v = 0xFF;
    ctr = 0;
    while(v != 0x00 && ctr < MAXTRIAL) {
	delay(1); /* wait one ms */
	v = cmd55(BASEPORT);
	if(v == 0xFF) {
	    continue;	/* try again on 0xFF */
	}
	v = acmd41(BASEPORT);
	ctr++;
    }
    if(ctr >= MAXTRIAL) {
	printf("Unable to send ACMD41 after %i attempts.\n", ctr);
	return -1;
    } else {
	printf("ACMD41 response: %02X (%i)\n", v, ctr);
    }

    /* CMD58 - READ OCR */
    v = cmd58(BASEPORT, rsp5);
    printf("CMD58 response: ");
    for(i=0; i<5; ++i) {
	printf("%02X ", rsp5[i]);
    }
    printf("\n");
    return 0;
}

void sd_set_miso_low(unsigned ioport) {
    outportb(ioport | 0x02, 0xFF);
}

void sd_set_miso_high(unsigned ioport) {
    outportb(ioport | 0x03, 0xFF);
}

void sd_disable(unsigned ioport) {
    sddis(ioport);
}