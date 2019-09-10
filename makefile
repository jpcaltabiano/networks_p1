all: http_client http_server

CXXFLAGS = -g -Wall -std=gnu99

http_client: http_client.o
	gcc $(CXXFLAGS) http_client.o -o http_client

http_client.o: http_client.c
	gcc -c $(CXXFLAGS) http_client.c

http_server: http_server.o
	gcc $(CXXFLAGS) http_server.o -o http_server

http_server.o: http_server.c
	gcc -c $(CXXFLAGS) http_server.c

clean:
	rm -f *.o http_client http_server
