CFLAGS=-O5 -D_OS_OS2 -Zexe
CC=gcc
AR=ar
MSQL=/public/msql2/lib
CGIDIR=/var/httpd/cgi-bin/dbadmin

all: libcgic.a dbadmin

libcgic.a: cgic.o cgic.h
	$(AR) rc libcgic.a cgic.o

dbadmin.o: dbadmin.c dbadmin.h msql.h
	$(CC) $(CFLAGS) -c dbadmin.c

dbadmin: dbadmin.o libcgic.a
	$(CC) $(CFLAGS) -o dbadmin dbadmin.o -s -L. -llibcgic -L$(MSQL) -llibmsql

clean:
	del *.o
	del *.a
	del dbadmin.

install:
	rm -rf $(CGIDIR)
	mkdir $(CGIDIR)
	cp dbadmin $(CGIDIR)
	chmod 4755 $(CGIDIR)/dbadmin
