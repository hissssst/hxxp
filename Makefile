BUILDDIR = build
CC = gcc
CFLAGS = -Wall -O2
OUT = /usr/local

install: all
	mkdir -p $(OUT)/bin
	mkdir -p $(OUT)/share/man/man1
	cp $(BUILDDIR)/* $(OUT)/bin
	gzip hxxp.1 -c > $(OUT)/share/man/man1/hxxp.1.gz

all: urlencode hxxp

hxxp:
	mkdir -p $(BUILDDIR)
	$(CC) src/hxxp.c -lcurl $(CFLAGS) -o $(BUILDDIR)/hxxp

urlencode:
	mkdir -p $(BUILDDIR)
	$(CC) src/urlencode.c -lcurl $(CFLAGS) -o $(BUILDDIR)/urlencode

clean:
	@rm -rf $(BUILDDIR)
