all: server cli td kd

# make rule per il server
server: server.c
	gcc server.c -o server -Wall

# make rule per il client
cli: cli.c
	gcc cli.c -o cli -Wall

# make rule per il table_device
td:	td.c
	gcc td.c -o td -Wall

# make rule per il kitchen_device
kd:	kd.c
	gcc kd.c -o kd -Wall

clean:
	rm *o server cli td kd