#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include "sdcmds.h"

/* DEFINE I/O PORTS */
#define BASEPORT  0x330
#define WRITEVAL  (BASEPORT | 0x00)
#define READVAL   (BASEPORT | 0x00)
#define STCLK     (BASEPORT | 0x01)
#define SDACT     (BASEPORT | 0x02)
#define SDDEACT   (BASEPORT | 0x03)

/* SD CARD COMMANDS */
static const unsigned char cmd17[] = {17|0x40, 0x00, 0x00, 0x00, 0x00, 0x01};

static void send_command(const unsigned char cmd[]) {
    int i;
    for(i=0; i<6; ++i) {
	outportb(WRITEVAL, cmd[i]);
	outportb(STCLK, 0xFF);
    }
    /*outportb(STCLK, 0xFF); /* send extra pulse for thinking */
}

static unsigned char receive_byte() {
    outportb(WRITEVAL, 0xFF);
    outportb(STCLK, 0xFF);
    return inportb(WRITEVAL);
}

static unsigned char buffer[512];
static unsigned checksum;

int main() {
    int i, j, ctr;
    unsigned char v, c1, c2;
    unsigned char *ptr;

    clrscr();
    printf("SDCARD interface program\n");

    /* TRY TO WAKE UP CARD */
    inportb(SDDEACT);
    sden(BASEPORT);		/* enable SD card */
    cmdclr(BASEPORT);           /* send reset pulses */
    v = cmd00(BASEPORT);	/* put card in idle state */
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
    printf("ACMD41 response: %02X (%i)\n", v, ctr);

    /* CMD58 - READ OCR */
    v = cmd58(BASEPORT);
    printf("CMD58 response: %02X ", v);
    for(i=0; i<4; ++i) {
	v = sdrecvf(BASEPORT);
	printf("%02X ", v);
    }
    printf("\n");

    /* CMD17 - BOOT SECTOR */
    send_command(cmd17);
    v = receive_byte();
    printf("CMD17 request: %02X\n", v);
    v = 0xFF;
    while(v != 0xFE) { /* wait for token byte */
	v = receive_byte();
    }
    ptr = buffer;
    for(i=0; i<512; ++i) {
	*(ptr++) = receive_byte();
    }
    c1 = receive_byte();
    c2 = receive_byte();
    checksum = (unsigned)c1 << 8 | (unsigned)c2;
    printf("Checksum: %04X\n", checksum);
    v = receive_byte();
    printf("Dummy byte: %02X\n", v);

    /* Test whether last two bytes of boot block are 0x55AA */
    if(buffer[510] == 0x55 && buffer[511] == 0xAA) {
	printf("Magic word identified: 0x55AA\n");
    } else {
	printf("Last two bytes do *not* match 0x55AA\n");
    }

    sddis(BASEPORT);	/* disable SD card */
    printf("Press any key to close\n");
    getch();

    return 0;
}