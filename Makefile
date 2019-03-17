CC=m68k-atari-mint-gcc
CFLAGS=--std=c99 -nostdlib -O2 -Wall

LIBCMINI=../libcmini/build

LIBS=-lgem -L$(LIBCMINI) -lcmini -lgcc

SRCS= \
	popup.c \
	nvlang.c

OBJS=$(patsubst %.c,%.o,$(SRCS))

nvlang.prg: $(OBJS)
	$(CC) $(CFLAGS) -s -o nvlang.prg $(LIBCMINI)/startup.o $(OBJS) $(LIBS)
	
.PHONY:clean
clean:
	rm -f *.o *~ core nvlang.prg

.PHONY:depend
depend: $(SRCS)
	-rm -f depend
	$(CC) $(CFLAGS) $(INCLUDE) -M $(SRCS) >> depend

ifneq (clean,$(MAKECMDGOALS))
-include depend
endif


