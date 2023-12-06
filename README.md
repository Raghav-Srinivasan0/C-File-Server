# C-File-Server

Transfer files over the internet using C! This is practice using C and its various features.

Requires nng library
Remember to make a test-dir directory and put the server files in there.
Remember to replace the ip address in the makefile to the servers

If you want to send files, compile with the command 'make' and then run the command ./server tcp://YOURIP:80 DIRECTORYNAME

Replace YOURIP with your ipv4 address (make sure to port forward it) and replace DIRECTORYNAME with a directory that you want to put files to transmit in. Then share your ip with the person recieving files. 
