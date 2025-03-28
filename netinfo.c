#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "netinfo.h"

void print_netif_single(const struct_netif ifs)
{
	printf("Interface %s\n", ifs.ifname);
	printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
											ifs.macaddr[0], \
											ifs.macaddr[1], \
											ifs.macaddr[2], \
											ifs.macaddr[3], \
											ifs.macaddr[4], \
											ifs.macaddr[5]);
	printf("Ip address : %d.%d.%d.%d \n",   ifs.ipaddr[0], \
											ifs.ipaddr[1], \
											ifs.ipaddr[2], \
											ifs.ipaddr[3]);
	printf("Broadcast : %d.%d.%d.%d \n",    ifs.bcast[0], \
											ifs.bcast[1], \
											ifs.bcast[2], \
											ifs.bcast[3]);
	printf("Subnet mask : %d.%d.%d.%d \n",  ifs.mask[0], \
											ifs.mask[1], \
											ifs.mask[2], \
											ifs.mask[3]);
	printf("\n");
}

void print_netif(const struct_netif *ifs, const int ifslen)
{
	for(int i=0; i<ifslen; i++)
	{
		printf("Interface INDEX %d - ", i);
		print_netif_single(ifs[i]);
	}
}

void parse_ioctl(const char *ifname, struct_netif *ni)
{
	int sock;
	struct ifreq ifr;
	struct sockaddr_in *ipaddr;
	size_t ifnamelen;


	/* copy ifname to ifr object */
	ifnamelen = strlen(ifname);
	if (ifnamelen >= sizeof(ifr.ifr_name)) {
		return ;
	}
	memcpy(ifr.ifr_name, ifname, ifnamelen);
	ifr.ifr_name[ifnamelen] = '\0';

	/* open socket */
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0) {
		return;
	}

	/* process mac */
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) != -1)
		memcpy(ni->macaddr, &ifr.ifr_hwaddr.sa_data, sizeof(ni->macaddr));

	/* die if cannot get address */
	if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
		shutdown(sock, 2);
		return;
	}

	/* process ip */
	ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	memcpy(ni->ipaddr, ((ifr.ifr_addr.sa_data) + 2), sizeof(ni->ipaddr));

	/* try to get broadcast */
	if (ioctl(sock, SIOCGIFBRDADDR, &ifr) != -1)
		memcpy(ni->bcast, ((ifr.ifr_broadaddr.sa_data) + 2), sizeof(ni->bcast));

	/* try to get mask */
	if (ioctl(sock, SIOCGIFNETMASK, &ifr) != -1)
		memcpy(ni->mask, ((ifr.ifr_netmask.sa_data) + 2), sizeof(ni->mask));

	shutdown(sock, 2);
}

int netif_search(const unsigned char *clientip, const struct_netif *ifs, const int ifslen)
{
	for(int i=0; i<ifslen; i++)
	{
		if( (ifs[i].ipaddr[0] & ifs[i].mask[0]) != 0)
		{
			for(int j=0; j<4; j++)
			{
				if( (ifs[i].ipaddr[j] & ifs[i].mask[j]) == (clientip[j] & ifs[i].mask[j]) )
				{
					if(j==3)
						return i;
				} else {
					break;
				}
			}
		}
	}
}

int init_interfaces(struct_netif *ni)
{
	DIR *d;
	struct dirent *de;
	int i;

	d = opendir("/sys/class/net/");
	if (d == NULL) {
		return -1;
	}

	i=0;
	while (NULL != (de = readdir(d))) {
		if (strcmp(de->d_name, ".") == 0 || \
			strcmp(de->d_name, "..") == 0 || \
			strcmp(de->d_name, "lo") == 0 )
		{
			continue;
		}

		strcpy(ni[i].ifname, de->d_name );
		parse_ioctl(ni[i].ifname, &ni[i]);
		i++;
	}

	closedir(d);

	return i;
}


#define MAXCHAR 100
int model_name(char *panel, const int slen)
{
	FILE *fp;
	char str[MAXCHAR];
	fp = fopen("/proc/device-tree/model", "r");
	if (fp == NULL){
		printf("Could not open file /proc/device-tree/model");
		return -1;
	}
	/* Read first line */
	fgets(str, MAXCHAR, fp);

	strncpy(panel, str, MAX(slen, strlen(str)) );

	fclose(fp);
	return 0;
}
