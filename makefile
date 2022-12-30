
all: amstwin amstkey testwin testkey

utf8len.o: utf8len.s
	as -o utf8len.o -g utf8len.s

amstwin: amstwin.c
	gcc -g -c -o amstwin.o amstwin.c

amstkey: amstkey.c utf8len.o
	gcc -g -c -o amstkey.o amstkey.c

testwin: testwin.c amstwin.o
	gcc -g -o testwin testwin.c amstwin.o

testkey: testkey.c amstkey.o
	gcc -g -o testkey testkey.c amstkey.o utf8len.o -lpthread

clean:
	rm utf8len.o amstwin amstkey
