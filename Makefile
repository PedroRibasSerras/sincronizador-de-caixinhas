all: clear server client
	echo 'feito'

server:
	gcc server.c -o server -lpthread

client:
	gcc `pkg-config --cflags libvlc` -c client.c -o client.o -lpthread
	gcc client.o -o client `pkg-config --libs libvlc` -lpthread

clear:
	if test -f "client";\
		then rm client;\
	fi
	if test -f "client.o";\
		then rm client.o;\
	fi
	if test -f "server";\
		then rm server;\
	fi
