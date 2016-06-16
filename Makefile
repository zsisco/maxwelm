CFLAGS+= -pedantic -Wall
LDADD+= -lX11 -lm
LDFLAGS=
EXEC=maxwelm

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

maxwelm: maxwelm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+ $(LDADD)

install: all
	install -Dm 755 maxwelm $(DESTDIR)$(BINDIR)/maxwelm

clean:
	rm -f maxwelm *.o

