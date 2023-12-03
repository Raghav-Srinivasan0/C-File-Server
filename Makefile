all: network_directory.h network_directory-client.c network_directory-server.c network_directory.c
	gcc network_directory.c network_directory-client.c -lnng -o client
	gcc network_directory.c network_directory-server.c -lnng -o server

run-client: client 
	sudo ./client tcp://172.24.148.15:80
run-server: server
	sudo ./server tcp://172.24.148.15:80