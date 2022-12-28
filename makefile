
all: amstwin amstkey

amstwin: amstwin.c
	gcc -g -o amstwin amstwin.c

amstkey: amstkey.c
	gcc -g -o amstkey amstkey.c -lpthread
