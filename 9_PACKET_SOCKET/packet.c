#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <net/if_arp.h>
#include <net/ethernet.h>

typedef struct arp_ha // hardware address
{
    unsigned char ha[6];
} arp_ha;

typedef struct arp_pa // protocol address
{
    unsigned char pa[4];
} arp_pa;

typedef struct arp_packet
{
    uint16_t ar_hrd; // destination
    uint16_t ar_pro; // origin
    uint8_t ar_hln;
    uint8_t ar_pln;
    arp_ha ar_sha;
    arp_pa ar_spa;
    arp_ha ar_tha;
    arp_pa ar_tpa;
};

int main()
{
    int pSocket;
    struct arp_packet *buff;
    int hdrLen;
    struct sockaddr_ll sockAddr;

}