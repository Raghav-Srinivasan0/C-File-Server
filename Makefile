all: network_directory.h network_directory-client.c network_directory-server.c network_directory.c
	gcc network_directory.c network_directory-client.c -lnng -lm -o client
	gcc network_directory.c network_directory-server.c -lnng -lm -o server

run-client: client 
	sudo ./client tcp://172.24.148.15:80 5519 41519438
run-server: server
	sudo ./server tcp://172.24.148.15:80 test-dir 7523 41519438