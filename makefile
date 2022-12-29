
all: amstwin amstkey

utf8len.o: utf8len.s
	as -o utf8len.o -g utf8len.s

amstwin: amstwin.c
	gcc -g -o amstwin amstwin.c

amstkey: amstkey.c utf8len.o
	gcc -g -o amstkey amstkey.c utf8len.o -lpthread

clean:
	rm utf8len.o amstwin amstkey
