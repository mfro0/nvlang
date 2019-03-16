CC=m68k-atari-mint-gcc
CFLAGS=-nostdlib -O2 -Wall

LIBCMINI=../libcmini/build

LIBS=-lgem -L$(LIBCMINI) -lcmini -lgcc

nvlang.acc: nvlang.c NVLANG.H
	$(CC) $(CFLAGS) -s -o nvlang.acc $(LIBCMINI)/startup.o nvlang.c $(LIBS)
	
clean:
	rm -f *.o *~ core nvlang.acc


