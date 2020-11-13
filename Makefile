Lab4 = server client

Obj = server.o client.o message.o
all: ${Lab4}
server: server.o message.o
	gcc server.o message.o -g -o server
client: client.o message.o
	gcc client.o message.o -g -o client
server.o: server.c
	gcc server.c -c -g -o server.o
client.o: client.c
	gcc client.c -c -g -o client.o
message.o: message.c
	gcc message.c -c -g -o message.o
clean:  
	rm -f ${Lab4} ${Obj}
