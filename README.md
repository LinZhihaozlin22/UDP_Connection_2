# UDP_Connection_2
Client using customized protocol on top of UDP protocol for requesting identification from server for access permission to the network.
(One client connects to one server.)

Assumption: 
If client receive a reject packet, it will print the rejected error and then keep sending the rest packets to server. (ack_timer for that packet will also stop after receiving reject response.)



---- Compile ---

 1. Open terminal, and open Assignment_2 folder using: "cd Assignment_2" 
    (may vary depend on the location of your files)
 2. then compile server file, enter: "gcc server.c -o server"
 3. then compile client file, enter: "gcc client.c -o client"

To use other port number: open "server.c" and "client.c" files and change the value of 'PORT' located at line 23 in "server.c" and line 29 in "client.c". Recompile if needed.



------ Run ----

 1. After compiling, run server using "./server". 
   (if success, a message "Listening for client messages..." will show)

 2. then open a new bash and enter the same directory. after that, run client using "./client"


  Initial test case is as following:
    packet 0 - simulate not paid
    packet 1 - simulate technology doesn't match
    packet 2 - simulate number doesn't found
    packet 3 - simulate permitted number
    packet 4 - simulate permitted number


To test ack_timer: run client using "./client" without running server (skip step 1 and do step 2)
