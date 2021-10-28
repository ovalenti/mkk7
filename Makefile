
PROJ?=custom

${PROJ}.k7:mkk7 ${PROJ}.bin
	./mkk7 ${PROJ}.bin ${PROJ}.k7

mkk7:mkk7.c

%.bin:%.ihx
	objcopy -I ihex -O binary $< $@

%.ihx:%.rel
	sdldz80 -i $<

%.rel:%.s
	sdasz80 -o $@ $<

.PRECIOUS:${PROJ}.rel ${PROJ}.ihx
