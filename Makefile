PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include maxwelm.c -L$(PREFIX)/lib -lX11 -o maxwelm

clean:
	rm -f maxwelm

