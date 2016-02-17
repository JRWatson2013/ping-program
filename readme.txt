Jacob Watson
JRWatson@wpi.edu
CS 4516: Advanced Computer Networks
Project 1 readme

How to compile:
To compile the program, type:

gcc proj1.c -lm -w -o proj1

DO NOT USE g++ OR LEAVE OUT -lm OR IT WILL NOT WORK

If you leave out -w, the program will compile and run, but gcc will generate warnings, which can be ignored.

To run the program, type:

sudo ./proj1 <TARGETWEBSITE/IP> <NUMBEROFTIMES>

WHERE:

<TARGETWEBSITE/IP>: The website or IP address to ping

<NUMBEROFTIMES> is the number of times to ping that site. THIS CAN BE LEFT OUT TO MAKE THE PROGRAM PING INDEFINATELY  

The program will then begin to ping the requested website or address. If this address is invalid or there is another error with setting up a connection, the program will inform you what went wrong, and then exit back to the command prompt. If everything works correctly, the program will attempt to ping the website. If a response is received within 1 second, the program will print out:

Received # byte reply from <TARGETWEBSITE/IP>: TTL: # icmp_seq: #, time: #

WHERE
the byte field is the size of the reply packet, TTL is the time to live, icmp_seq is the icmp protocol sequence number of the response packet, and time is the time in miliseconds it took for the response to the ping to return.

If more than one second goes by without a response, the ping will time out, and the program will print:

Request timed out.

The program will then move on to the next step.

The program will either repeat this process the number of times it was requested on the command line, or if it was set to infinity, until the user types

control-c

In either case, once control-c or the requested ping count is reached, the program will print out total statistics about the ping session, which includes:

# packets sent, # packets recieved, # packet loss
min time: #, max time #, average: #, stddev: # 

where packets sent is the number of packets sent, packets recieved is the number of responses that were returned to the host, packet loss is the percentage of pings that did not have a response, min time is the smallest amount of time it took to get a response from the target, max time is the largest amount of time it took to get a response from the target, average is the average response time from the target, and stddev is the standard deviation for the response time.

The program will then end.

Requirements:

This program runs on Ubuntu Linux 14.04 LTS 64 bit or 32 bit. 
It compiles with gcc version 4.8.2 or 4.8.4.
It uses the Ubuntu EGLIBC 2.19-0ubuntu6.6) 2.19 C Library.

It will run on any physical or virtual machine that meets these requirements. 

The program will compile and run with the above OS, compiler, and runtime.
Other setups have not been tested and come with no guarantee.

NOTES:
g++ will not work. running the command without sudo will not work.
leaving out -lm on the gcc command will not work.


Step by step:

1.Compile the program with 

gcc prog1.c -lm -w -o proj1

2.Run the program with

sudo ./proj1 <TARGETWEBSITE/IP> <TIMESTORUN>

3.Enter your password if prompted by the computer 

4.The program will ping the target for the amount of times requested or until you hit control-c. The result for each ping will be printed out to the screen.

5. The Final summary results for the session will be printed out. The program will end.
