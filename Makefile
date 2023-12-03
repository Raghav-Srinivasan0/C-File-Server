all: network_directory.h network_directory-client.c network_directory-server.c network_directory.c
	gcc network_directory.c network_directory-client.c -lnng -o client
	gcc network_directory.c network_directory-server.c -lnng -o server

run-client: client 
	sudo ./client 
run-server: server
	sudo ./server