/*
Jacob Watson
JRWatson@wpi.edu
CS 4516: Advanced Computer Networks
Project 1

Used guides located at:
http://www.binarytides.com/get-local-ip-c-linux/
http://www.linuxhowtos.org/C_C++/socket.htm
http://people.cis.ksu.edu/~neilsen/cis525/programs/pinger.c
http://tcpip.marcolavoie.ca/ethernet_frame.html
https://www.cs.utah.edu/~swalton/listings/sockets/programs/part4/chap18/ping.c

*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

struct timeval timer; //used to time packet timeouts
struct timeval starttime; //packet sent time
struct timeval endtime; //packet received time
int startsec, startmac; //start time values
int endsec, endmac; //end time values
int startmili, endmili; //start and end times in miliseconds
int calcfinal; //packet transmission time in miliseconds
int maxtime = 0; //holds longest packet trans time
int mintime = 99999999; //holds shortest packet trans time
float total = 0; //holds total trans time
float average = 0; //holds average trans time
float sttdev = 0; //holds sttdev time
float timearray[1000000]; //holds the trans time for each ping to be used for calcuations
float difference; //difference
double  variance; //variance
float runningtotal; //running total for the variance before it is calcluated

char destination[20]; //the target ip/website
int trycount; //how many times to ping
float packetssent = 0; //the number of packets sent
float packetsrecv = 0; //the number of packets received
float packetsgot = 0; //used to calculate packet loss
float packetloss = 0; //holds the packet loss
unsigned short checksum(unsigned short *, int); //calculates the checksum
void readin(char**, char*); //used to read in from the command line
char* convert(char*); //converts the destination into an ip address that can be pinged

int fd; //file discriptor to be used for the socket this program will ping with

void sigintHandler(int sig_num) //this routine is called when control c is hit. It displays stats and ends the program
{
  int loopcnt; //loop iteratior
  
  signal(SIGINT, sigintHandler);

  packetsgot = (packetsrecv / packetssent); //get the # of packets received
  packetloss = (1 - packetsgot); //get the # of packets lost
  average = (total / packetsrecv); //get the average
  for (loopcnt = 0; loopcnt < packetsrecv; loopcnt++) //loop through the array of trans times
    {
      difference = (timearray[loopcnt] - average) * (timearray[loopcnt] - average); //get the difference for this time and the average
      runningtotal = (difference + runningtotal); //add it to the running total
    }
  variance = (runningtotal / packetsrecv); //calculate the variance
  sttdev = sqrt(variance); //calculate the standard deviation
  if(mintime ==  99999999) {
      mintime = 0; //if no packets were recieved, set the mintime to a default of 0, because 9999999 isn't a good default to display
  }
  printf("\n %s statistics: \n %.0f packets sent, %.0f packets received, %f packet loss \n", destination, packetssent, packetsrecv, packetloss);
  printf(" min time: %d, max time: %d, average: %f, stddev: %f \n", mintime, maxtime, average, sttdev); //print out the stats
  close(fd); //close the file discriptor
  exit(0); //exit the program
}


//the main function handles the setup and actual pinging. If control-c is not hit, it also handles the statistics
int main(int argc, char* argv[])
{

  signal(SIGINT, sigintHandler); 
  struct iphdr* ip; //pointer used to set ip data
  struct iphdr* response; //pointer used to view ip data
  struct icmphdr* icmp; //pointer used to change icmp data
  struct icmphdr *icp; //pointer used to view icmp data
    struct sockaddr_in connectinfo; //socket address information
  char* packet; //packet buffer
  char* buffer; //return packet buffer
  int optimumvalue; //optimum value
  int addresslength; //holds the length of the address
  int size; //holds the size of the received packet


  if(getuid() != 0) {
    printf("Run this command as root.\n"); //user didnt type sudo on command line. Can't do anything more.
    exit(1); //quit
  }
  int looper; //used for array loop
  for(looper = 0; looper < 1000000; looper++)
    {
      timearray[looper] = 0; //initialize the array
    }
  readin(argv, destination); //get the destination information and the run count information. 
  strncpy(destination, convert(destination), 20); //convert the target info into a ip address
  printf("Sending ping to %s.\n", destination); // inform the user

  const char* dnsaddress = "8.8.8.8"; //we will use this ip address, googles DNS server, to figure out our IP
  int dnsport = 53; //dns port

  struct sockaddr_in dnsserver; //information for the socket we will use to figure out the source IP

  int dnssock = socket(AF_INET, SOCK_DGRAM, 0); //set up the google DNS socket

  if(dnssock < 0) {
    printf("Couldn't create DNS socket. \n Can't get hostname. \n"); //if it didn't work, we can't do anything else. 
    exit(1); //exit
  }
 
  //in the below commands, set up the dnsserver socket address info
  memset(&dnsserver, 0, sizeof(dnsserver));
  dnsserver.sin_family = AF_INET;
  dnsserver.sin_addr.s_addr = inet_addr(dnsaddress);
  dnsserver.sin_port = htons(dnsport);

  int failcheck = connect(dnssock, (const struct sockaddr*) &dnsserver, sizeof(dnsserver)); //use the socket info to connect to 8.8.8.8

  struct sockaddr_in dnsname; //create another socket address structure to hold the source address we will pull out of the socket
  socklen_t namelength = sizeof(dnsname); //set it up
  failcheck = getsockname(dnssock, (struct sockaddr*) &dnsname, &namelength); //pull out the source address from the socket

  char buffersource[100]; //set up a buffer to hold the source address
  const char* source = inet_ntop(AF_INET, &dnsname.sin_addr, buffersource, 100); //set up a pointer to the source address buffer, convert it to ip

  if(source != NULL){ //if it worked right...
    printf("Source IP: %s \n", buffersource); //... print out the source ip
  } else { //or if it failed...
    printf("Error getting source IP. \n"); //...inform the user we can't continue
    exit(1);//exit
  }

  close(dnssock); //we dont need that google dns socket anymore, so close it down

  packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr)); //initialize the packet buffer
  buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr)); //initialize the return buffer

  ip = (struct iphdr*) packet; //set the ip pointer to point to the packet buffer
  icmp = (struct icmphdr*) (packet + sizeof(struct iphdr)); //set up the icmp pointer to point to the icmp part of the packet buffer

  //insert all the settings for the ip protocol into the packet
  ip->ihl = 5;
  ip->version = 4;
  ip->tos = 0;
  ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
  ip->id = htons(0);
  ip->frag_off = 0;
  ip->ttl = 64;
  ip->protocol = IPPROTO_ICMP;
  ip->saddr = inet_addr(source);
  ip->daddr = inet_addr(destination);
  ip->check = checksum((unsigned short *)ip, sizeof(struct iphdr));
  

  if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) //try to create the socket we will use to send the ping
    {
      printf("There was an error creating the socket.\n Try again. \n"); //if the socket cant be created, we cant do anything more
      exit(1); //exit
    }

  setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &optimumvalue, sizeof(int)); //set the ip settings on the socket
  //set the timer structure, used for time outs, to one second
  timer.tv_sec = 1;
 timer.tv_usec = 0;

 setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timer, sizeof(timer)); //set the time out time on the socket
 //set the icmp settings in the packet buffer
  icmp->type = ICMP_ECHO;
  icmp->code = 0;
  icmp->un.echo.id= 0;
  icmp->un.echo.sequence = 0; 
  icmp ->checksum = checksum((unsigned short *)icmp, sizeof(struct icmphdr)); //calculate checksum

  //set the family and destination in the socket information
  connectinfo.sin_family = AF_INET; 
  connectinfo.sin_addr.s_addr = inet_addr(destination);
  int i = 0; //set up i as a loop iterator
  while(i != trycount){ //loop until all pings are complete or indefinately if trycount is set to -1
  gettimeofday(&starttime, NULL); //mark time packet is sent
  sendto(fd, packet, ip->tot_len, 0, (struct sockaddr *) &connectinfo, sizeof(struct sockaddr)); //send out the packet
  packetssent++; //count the sent packet in the total
  printf("Sending ping ... \n"); //inform the user

  addresslength = sizeof(connectinfo); //get the address length
  //try to receive a response. Wait for a second before timing out.
  if((size = recvfrom (fd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0, (struct sockaddr *)&connectinfo, &addresslength)) == -1) {
    printf ("Request timed out. \n"); //If a second went by without a response, time out and move on
    i++; //increment the loop counter
  } else { //if a response was received
    gettimeofday(&endtime, NULL); //mark the time a response to the ping was received
    sleep(1); //wait for a second to slow down the packet flow
    printf("Received %i byte reply from %s :",size, destination); //let the user know about the return packet and its size
    response = (struct iphdr*) buffer; //point response to the response packet buffer ip section
    icp = (struct icmpdhr*) (buffer + (response->ihl * 4)); //point icp to the response packet buffer icmp section
    printf("TTL: %d ", response->ttl); //print out the TTL
    printf("icmp_seq: %u ", icp->un.echo.sequence); //print out the icmp_seq for the received packet
    i++; //increment the loop counter
    startmac = (starttime.tv_usec / 1000); //convert the start microseconds to miliseconds
    startsec = (starttime.tv_sec * 1000); //convert the start seconds to miliseconds
    startmili = startmac + startsec; // total up the start time
    endmac = (endtime.tv_usec / 1000); //convert the end microseconds to miliseconds
    endsec = (endtime.tv_sec * 1000); //convert the end seconds to miliseconds
    endmili = endmac + endsec; //total up the end time
    calcfinal = endmili - startmili; //figure out how long in miliseconds the ping took
    printf("time: %d ms \n", calcfinal); //print out the trans time
    total = total + calcfinal; //add trans time to running count
    if(calcfinal < mintime) { 
      mintime = calcfinal; //if this is the smallest trans time so far, it is the current minimum
    }
    if(calcfinal > maxtime) {
      maxtime = calcfinal; //if this is the greatest trans time so far, it is the current maximum
    }
    timearray[(int) packetsrecv] = calcfinal; //add the trans time to the array
    packetsrecv++; //increase the total of received packets
  }
  }
  int loopcnt; //loop iterator
  /*
THIS SECTION IS THE SAME AS THE SIGINTHANDLER ROUTINE. TO SEE COMMENTS ON IT, LOOK THERE.
  */

  packetsgot = (packetsrecv / packetssent);
  packetloss = (1 - packetsgot);
  average = (total / packetsrecv);
  for (loopcnt = 0; loopcnt < packetsrecv; loopcnt++)
    {
      difference = (timearray[loopcnt] - average) * (timearray[loopcnt] - average);
      runningtotal = (difference + runningtotal);
    }
  variance = (runningtotal / packetsrecv);
    sttdev = sqrt(variance);
    if(mintime ==  99999999) {
      mintime = 0; //if no packets were recieved, set the mintime to a default of 0, because 9999999 isn't a good default to display
  }
  printf("\n %s statistics: \n %.0f packets sent, %.0f packets received, %f packet loss \n", destination, packetssent, packetsrecv, packetloss);
  printf(" min time: %d, max time: %d, average: %f, stddev: %f \n", mintime, maxtime, average, sttdev);
  free(packet);
  free(buffer);
  close(fd);
  return 0;

}

//this routine reads in the command line arguements 
void readin(char** argv, char* destin) {

  if(!(*(argv + 1))) //if there are no command line arguments
    {
      printf("incorrect input \n");
      exit(1);
    }
  if(*(argv + 1) && (!(*(argv + 2)))) { // if there is only a destination on the command line
    strncpy(destin, *(argv + 1), 15); //copy the destination
    trycount = -1; //set the trycount to -1, so the ping loop will never terminate
    return;
  }
  else if ((*(argv + 1) && (*(argv + 2)))) //if both arguements are defined
    {
      strncpy(destin, *(argv + 1), 15); //copy over the destination
      trycount = atoi(*(argv + 2)); //copy over the ping count
      return;
    }
}

char* convert(char* address){ //convert the destination to an ip address
  struct hostent* host; //holds the destinations host information
  host = gethostbyname(address); //get the destinations host information
  return inet_ntoa(*(struct in_addr *)host->h_addr); //pass back the actual destination ip address
}

unsigned short checksum(unsigned short *address, int length){ //calcuate a checksum
  register int total = 0;
  u_short ans = 0;
  register u_short *w = address; //address
  register int remain = length; //address length

  while (remain > 1) //sum up 2 byte values
    {total += *w++;
      remain -= 2;
    }
  if(remain == 1) //handle remainder
    {
      *(u_char *) (&ans) = *(u_char *) w;
      total += ans;
    }
  //fold in 32-bit sum into 16 bits
  total = (total >> 16) + (total & 0xffff);
  total += (total >> 16);
  ans = ~total;
  return (ans); //return the calcuated checksum
}

