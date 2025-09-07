	.model small,C
	.data
;-------------------------------------------------------------------------
dcmd00  db 40h, 00h, 00h, 00h, 00h, 95h
dcmd08	db 48h, 00h, 00h, 01h,0AAh, 87h
dcmd55	db 77h, 00h, 00h, 00h, 00h, 65h
dacmd41	db 69h, 40h, 00h, 00h, 00h, 77h
dcmd58	db 7Ah, 00h, 00h, 00h, 00h, 01h
;-------------------------------------------------------------------------
	.code
	public cmdclr           ; send 10 blank pulses to SD card
	public sden		; pull /CS low, selecting SD card
	public sddis		; pull /CS high, deselecting SD card
	public cmd00		; send command cmd00
	public cmd08		; send command cmd08
	public cmd55		; send command cmd55 (advanced command)
	public acmd41		; send command acmd41
	public cmd58		; send command cmd58
	public sdrecvf		; receive single byte from SD

;-------------------------------------------------------------------------
; send 10 blank pulses to SD card
;-------------------------------------------------------------------------
cmdclr  proc
	push bp			; put bp on stack
	mov bp,sp		; put stack in bp
	mov dx, [bp+4]		; store base I/O port
	mov al, 0FFh
	out dx, al		; set shift register
	inc dx			; set startclk command
	mov cx, 10		; number of iterations
l1:	out dx, al		; keep pulsing clock
	loop l1
	pop bp
	ret
cmdclr  endp
;-------------------------------------------------------------------------
; SD command CMD00
;-------------------------------------------------------------------------
cmd00  	proc
	push bp
	mov bp,sp
	mov dx, [bp+4]
	push si
	mov si,OFFSET dcmd00		; set pointer
	call sendcmd
	pop si
	pop bp
	ret
cmd00	endp
;-------------------------------------------------------------------------
; SD command CMD08
;-------------------------------------------------------------------------
cmd08  	proc
	push bp
	mov bp,sp
	mov dx, [bp+4]
	push si
	mov si,OFFSET dcmd08		; set pointer
	call sendcmd
	pop si
	pop bp
	ret
cmd08	endp
;-------------------------------------------------------------------------
; SD command CMD55
;-------------------------------------------------------------------------
cmd55  	proc
	push bp
	mov bp,sp
	mov dx, [bp+4]
	push si
	mov si,OFFSET dcmd55		; set pointer
	call sendcmd
	pop si
	pop bp
	ret
cmd55	endp
;-------------------------------------------------------------------------
; SD command ACMD41
;-------------------------------------------------------------------------
acmd41 	proc
	push bp
	mov bp,sp
	mov dx, [bp+4]
	push si
	mov si,OFFSET dacmd41		; set pointer
	call sendcmd
	pop si
	pop bp
	ret
acmd41	endp
;-------------------------------------------------------------------------
; SD command CMD58
;-------------------------------------------------------------------------
cmd58  	proc
	push bp
	mov bp,sp
	mov dx, [bp+4]
	push si
	mov si,OFFSET dcmd58		; set pointer
	call sendcmd
	pop si
	pop bp
	ret
cmd58	endp
;-------------------------------------------------------------------------
; General send command, assumes DX contains BASE I/O address
;-------------------------------------------------------------------------
sendcmd	proc
	cld			; set incrementing direction
	mov cx, 6		; set number of bytes
l2:	lodsb			; load byte from SI into AL and incr SI
	;call puthex
	out dx, al		; put value in register
	inc dx			; increment DX for startclk I/O address
	out dx, al		; start clk
	dec dx			; decrement DX for base I/O address
	loop l2
	mov cx, 100		; set counter
check:	inc dx			; reset to clk start
	out dx, al		; pulse
	dec dx			; set to retrieve
	in al, dx		; retrieve byte
	cmp al,0FFh		; check if equal to 0xFF
	jne endrun		; if not, return value
	dec cx			; if so, decrement trial counter
	jz endrun		; end if trial counter equal to zero
	jmp check		; else try again
endrun: ret
sendcmd	endp
;-------------------------------------------------------------------------
; Receive byte from SD card; assumes register has 0xFF
;-------------------------------------------------------------------------
sdrecvf	proc
	push bp
	mov bp,sp
	mov dx, [bp+4]
	mov bx, 0
l3:	inc dx
	out dx, al		; send empty bytes
	dec dx
	in al, dx		; receive byte
	cmp al, 0FFh
	jne endrcv
	inc bx
	cmp bx, 10
	jne l3
endrcv:	pop bp
	ret
sdrecvf	endp
;-------------------------------------------------------------------------
; enable SD card by pulling /CS low
;-------------------------------------------------------------------------
sden    proc
	push bp
	mov bp,sp
	mov dx, [bp+4]		; set base I/O port
	add dx, 2		; set act port BASE | 02
	mov al, 0FFh		; some dummy value
	out dx, al              ; and send I/O instruction
	pop bp
	ret
sden    endp
;-------------------------------------------------------------------------
; disable SD card by puling /CS high
;-------------------------------------------------------------------------
sddis   proc
	push bp
	mov bp,sp
	mov dx, [bp+4]		; set base I/O port
	add dx, 3		; set deact port BASE | 02
	mov al, 0FFh		; some dummy value
	out dx, al		; and send I/O instruction
	pop bp
	ret
sddis   endp
;-------------------------------------------------------------------------
; Output value in AL to the screen
; Conserves all
;-------------------------------------------------------------------------
puthex  proc
	push ax
	push dx
	mov dh,al	; store copy
	shr al,1	; shift upper nibble
	shr al,1
	shr al,1
	shr al,1
	call putnib
	mov al,dh
	and al,0Fh
	call putnib
	mov dl,' '
	mov ah,02h
	int 21h
	pop dx
	pop ax
	ret
puthex  endp
putnib  proc
	add al,'0'	; set 0-9 range
	cmp al,'9'	; check if in range
	jbe skip	; if so, print it
	add al,7	; else set A-F range
skip:	mov dl,al
	mov ah,02h
	int 21h
	ret
putnib  endp
	end
