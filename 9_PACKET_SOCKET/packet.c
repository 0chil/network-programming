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
    uint16_t ar_op;
    arp_ha ar_sha;
    arp_pa ar_spa;
    arp_ha ar_tha;
    arp_pa ar_tpa;
};

struct sockaddr_ll
{
    unsigned short sll_family;   /* Always AF_PACKET */
    unsigned short sll_protocol; /* Physical-layer protocol */
    int sll_ifindex;             /* Interface number */
    unsigned short sll_hatype;   /* ARP hardware type */
    unsigned char sll_pkttype;   /* Packet type */
    unsigned char sll_halen;     /* Length of address */
    unsigned char sll_addr[8];   /* Physical-layer address */
};

int main()
{
    int pSocket;             // 패킷을 날릴 소켓
    struct arp_packet *buff; // 날릴 패킷
    struct sockaddr_ll sockAddr; // 날릴 주소 정보
    int hdrLen;
    int res, i;
    int tempInt;
    unsigned char test;

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sll_family = AF_PACKET;
    sockAddr.sll_protocol = htons(ETH_P_ARP);
    sockAddr.sll_ifindex = if_nametoindx("eth0");
    sockAddr.sll_halen = ETH_ALEN;
    sockAddr.sll_addr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    pSocket = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
    buff = (struct arp_packet *)malloc(sizeof(struct arp_packet));
    buff->ar_hrd = htons(0x0001);
    buff->ar_pro = htons(0x0800);
    buff->ar_hln = 6;
    buff->ar_pln = 4;
    buff->ar_op = htons(0x0001); // 0x0001 요청 0x0002 응답
    convertTextToArpHa("xx:xx:xx:xx:xx:xx", &(buff->ar_sha));
    convertTextToArpPa("111.111.111.111", &(buff->ar_spa));
    convertTextToArpPa("xxx.xxx.xxx.xxx", &(buff->ar_tpa));

    sendto(pSocket, buff, hdrLen, 0, &sockAddr, sizeof(sockAddr));
    close(pSocket);
}