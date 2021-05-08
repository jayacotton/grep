CFLAGS	= +cpm -Wall -pragma-include:zpragma.inc
#CFLAGS	= +cpm -Wall -DNEW -pragma-include:zpragma.inc
#CFLAGS	= +cpm -Wall --list --c-code-in-asm -pragma-include:zpragma.inc
LINKOP	= +cpm -create-app -m  -pragma-include:zpragma.inc
DESTDIR = ~/HostFileBdos/c/
DESTDIR1 = /var/www/html
SUM = sum
CP = cp
INDENT = indent -kr
SUDO = sudo

# define SNAP to null when debugging is done.
SNAP =
#SNAP =	snaplib.o

all: grep

grep: grep.o snaplib.o wildexp.o tools.o
	zcc $(LINKOP) -ogrep grep.o wildexp.o tools.o $(SNAP)

wildexp.o: wildexp.c
	zcc $(CFLAGS) -c wildexp.c

grep.o: grep.c
	date > date.h
	zcc $(CFLAGS) -c grep.c

snaplib.o: snaplib.c
	zcc $(CFLAGS) -c snaplib.c

tools.o: tools.c tools.h
	zcc $(CFLAGS) -c tools.c

clean:
	$(RM) *.o *.err *.lis *.def *.lst *.sym *.exe *.COM  *.map grep

just:
	$(INDENT) grep.c
	$(INDENT) tools.c
	$(INDENT) wildexp.c

scope:
	cscope

install:
	$(SUDO) $(CP) ./*.COM $(DESTDIR1)/. 
	$(CP) GREP.COM $(DESTDIR)grep.com

check:
	$(SUM) *.COM

