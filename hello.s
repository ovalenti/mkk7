; "Hello world" example
	.area CODE (ABS) ; ABS will allow for a non-relocatable object
	.org 0x4A0C

	ld bc, #str
	ld hl, #0x4232 ; screen buffer location
loop:
	ld a, (bc)
	or a, a
	jr Z, out
	ld (hl), a
	inc hl ; character flags
	ld (hl), #0
	inc hl
	inc bc
	jr loop

out:
	; instruct the interrupt routine to refresh the screen
	ld hl, #0x47fb
	ld (hl),#1

end:
	halt
	jr end
str:
	.ascii "hello!"
	.db 0x00

