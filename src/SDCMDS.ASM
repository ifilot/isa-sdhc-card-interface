	.model small,C
	.data
	.code
	public cmdclr           ; send 10 blank pulses to SD card
	public sden		; pull /CS low, selecting SD card
	public sddis		; pull /CS high, deselecting SD card

;-------------------------------------------------------------------------
; send 10 blank pulses to SD card
;-------------------------------------------------------------------------
cmdclr  proc

	push bp			; put bp on stack
	mov bp,sp		; put stack in bp
	mov dx, [bp+4]		; store base I/O port

	mov al, 255
	out dx, al		; set shift register

	inc dx			; set startclk command
	mov cx, 10		; number of iterations
l1:	out dx, al		; keep pulsing clock
	loop l1

	pop bp
	ret

cmdclr  endp
;-------------------------------------------------------------------------
; enable SD card
;-------------------------------------------------------------------------
sden    proc
	push bp
	mov bp,sp
	mov dx, [bp+4]		; set base I/O port
	add dx, 2		; set act port BASE | 02
	mov al, 255		; some dummy value
	out dx, al              ; and send I/O instruction
	pop bp
	ret
sden    endp
;-------------------------------------------------------------------------
; distable SD card
;-------------------------------------------------------------------------
sddis   proc
	push bp
	mov bp,sp
	mov dx, [bp+4]		; set base I/O port
	add dx, 3		; set deact port BASE | 02
	mov al, 255		; some dummy value
	out dx, al		; and send I/O instruction
	pop bp
	ret
sddis   endp


	end
