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
static const unsigned char cmd00[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
static const unsigned char cmd08[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
static const unsigned char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0x65};
static const unsigned char acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0x77};
static const unsigned char cmd58[] = {58|0x40, 0x00, 0x00, 0x00, 0x00, 0x01};
static const unsigned char cmd17[] = {17|0x40, 0x00, 0x00, 0x00, 0x00, 0x01};

static void send_command(const unsigned char cmd[]) {
    int i;
    for(i=0; i<6; ++i) {
	outportb(WRITEVAL, cmd[i]);
	outportb(STCLK, 0xFF);
    }
    outportb(STCLK, 0xFF); /* send extra pulse for thinking */
}

static unsigned char receive_byte() {
    outportb(WRITEVAL, 0xFF);
    outportb(STCLK, 0xFF);
    return inportb(READVAL);
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

    /* PUT CARD IN IDLE STATE */
    send_command(cmd00);
    v = receive_byte();
    printf("Received: %02X\n", v);

    /* SEND COMMAND 08 */
    send_command(cmd08);

    for(i=0; i<5; ++i) {
	v = receive_byte();
	printf("%02X", v);
    }
    printf("\n");

    /* TRY ACMD41 */
    v = 0xFF;
    ctr = 0;
    while(v != 0x00 && ctr < 100) {
	send_command(cmd55);
	v = receive_byte();
	send_command(acmd41);
	v = receive_byte();
	ctr++;
    }
    printf("ACMD41: %02X (%i)\n", v, ctr);

    /* CMD58 - READ OCR */
    send_command(cmd58);
    for(i=0; i<5; ++i) {
	v = receive_byte();
	printf("%02X", v);
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