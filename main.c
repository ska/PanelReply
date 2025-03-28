/*
 * udpserver.c - A simple UDP echo server
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include "netinfo.h"

#define BUFSIZE 256

typedef struct
{
	char identification[10];
	char opcode[2];
	char hardware[16];
	char ip[16];
	char netmask[16];
	char RemoteConfPort[4];
	char moduleName[5];
	char manufactureCode[2];
	char reserved[12];
	char hostName[64];
} struct_selfinfo;

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char **argv)
{
	int sockfd;						/* socket */
	int portno;						/* port to listen on */
	socklen_t clientlen;			/* byte size of client's address */
	struct sockaddr_in serveraddr;	/* server's addr */
	struct sockaddr_in clientaddr;	/* client addr */

	char *buf;						/* message buf */
	int optval;						/* flag value for setsockopt */
	int n;							/* message byte size */
	struct_selfinfo sfi;
	char hostname[64];
	unsigned char clientip[4];

	struct_netif ifs[10];
	int ifslen = init_interfaces(ifs);
	//printf("Found %d interfaces\n", ifslen);
	//print_netif(ifs, ifslen);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

	/* build the server's Internet address */
	portno = 991;
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	/* bind: associate the parent socket with a port */
	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		error("ERROR on binding");

	/* main loop: wait for a datagram, then echo it */
	clientlen = sizeof(clientaddr);
	while (1) {

		/* recvfrom: receive a UDP datagram from a client */
		buf = malloc(BUFSIZE);
		n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);
		if (n < 0)
			error("ERROR in recvfrom");

		/* gethostbyaddr: determine who sent the datagram */
		gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
							  sizeof(clientaddr.sin_addr.s_addr),
							  AF_INET);

		clientip[3] = ((clientaddr.sin_addr.s_addr & 0xFF000000) >> 24);
		clientip[2] = ((clientaddr.sin_addr.s_addr & 0x00FF0000) >> 16);
		clientip[1] = ((clientaddr.sin_addr.s_addr & 0x0000FF00) >> 8);
		clientip[0] = ((clientaddr.sin_addr.s_addr & 0x000000FF) >> 0);
		//fprintf(stdout, "Received %d bytes data %d.%d.%d.%d\n", n, clientip[0], clientip[1], clientip[2], clientip[3]);

		int found = netif_search(clientip, ifs, ifslen);
		//printf("Received on IF[%d]: %s\n", found, ifs[found].ifname );

		gethostname(hostname, sizeof(hostname));
		memset(&sfi, 0x20, sizeof(struct_selfinfo));
		strncpy(sfi.identification, "I am here.", 10);
		strncpy(sfi.opcode,			"03", 2);
		sprintf(sfi.hardware, "%02X%02X%02X%02X%02X%02X", \
								ifs[found].macaddr[0], \
								ifs[found].macaddr[1], \
								ifs[found].macaddr[2], \
								ifs[found].macaddr[3], \
								ifs[found].macaddr[4], \
								ifs[found].macaddr[5]);
		sprintf(sfi.ip, "%d.%d.%d.%d", \
								ifs[found].ipaddr[0], \
								ifs[found].ipaddr[1], \
								ifs[found].ipaddr[2], \
								ifs[found].ipaddr[3]);
		sprintf(sfi.netmask, "%d.%d.%d.%d", \
								ifs[found].mask[0], \
								ifs[found].mask[1], \
								ifs[found].mask[2], \
								ifs[found].mask[3]);

		model_name(sfi.moduleName, sizeof(sfi.moduleName));

		strncpy(sfi.RemoteConfPort,	"####", strlen("####"));
		strcpy(sfi.manufactureCode, "*");
		strcpy(sfi.hostName,		hostname);
		memcpy(buf, &sfi, sizeof(struct_selfinfo));

		n = sendto(sockfd, buf, 148, 0,
				   (struct sockaddr *)&clientaddr, clientlen);

		if (n < 0)
			error("ERROR in sendto");
	}
}
