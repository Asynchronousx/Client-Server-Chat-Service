# simple-client-server-chat

°---------------------------------------°
|      written by Asynchronousx         |
°---------------------------------------°

Files:
--------
1. Server.c - server side of the application
2. Client.c - client side of the application

Description:
--------
This simple chat application is made by using socket programming in C.
This is a 1:1 chat multimessaging service. The server will act as a client, 
waiting for a client to connect, and chatting directly to him. 
The client will wait the server response, then begin to chat with the server too.
To exit the service, just type /EXIT and press enter.

Usage:
--------
If you want to test it in local, just copypaste both Server & Client and compile it
in a terminal.
When running with *./Client* and *./Server* respectively, we need to pass as first argument 
the same port for both, to let them communicate: 
An example could be ./Server 4000 - ./Client 4000.
If you want to test it from different computer and network, change the loopback address 
in _Client.c_ marked by "127.0.0.1" with your machine server IP, to let the client 
reach the server on a different machine.

