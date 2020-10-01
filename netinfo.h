#ifndef NETINFO_H
#define NETINFO_H

typedef struct
{
        unsigned char ifname[10];
        unsigned char macaddr[6];
        unsigned char ipaddr[4];
        unsigned char bcast[4];
        unsigned char mask[4];
} struct_netif;


int init_interfaces(struct_netif *ni);
void print_netif(const struct_netif *ifs, const int ifslen);
int netif_search(const unsigned char *clientip, const struct_netif *ifs, const int ifslen);

int model_name(char *panel);

#endif // NETINFO_H
