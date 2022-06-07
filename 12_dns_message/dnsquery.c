#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define A 1
#define AAAA 28
#define MX 15
#define TXT 16
#define CNAME 5

void errproc(const char *func)
{
    fprintf(stderr, "error on : %s\n", func);
    exit(1);
}

const u_char *print_name(const u_char *msg, const u_char *p, const u_char *end)
{
    if (p + 2 > end)
    { // 시작과 끝은 2차이가 나야함. Length 1-byte, Value 1-byte 일떄에도 최소 2-byte는 필요함
        errproc("print_name, Unexpected End of Message");
    }

    if ((*p & 0xC0) == 0xC0) // xxxx xxxx & 1100 0000 == 1100 0000 (Compression set)
    {
        const int k = ((*p & 0x3F) << 8) + p[1]; // ((xxxx xxxx & 0011 1111) << 8 + xxxx xxxx) = 00xx xxxx xxxx xxxx
        p += 2;                                  // 다음 읽을 주소로 이동(현재 2-bytes 를 읽었음)
        printf("(pointer %X)", k);
        print_name(msg, msg + k, end);
        return p;
    }
    else // no compression bit set
    {
        const int len = *p; // xxxx xxxx -> 정수로 바로 넣기(Signed Extension)
        p++;                // length 바이트를 읽었으므로 넘어감
        /*
        length 1, value w라고 하자.
            p       end
        | 1 | w | 0 |
        이렇다고 하더라도, {p + (length = 1) + (Termination(0) = 1)} 이 end를 넘어가서는 안된다.
         */
        if (p + len + 1 > end)
        {
            errproc("print_name, with no compression bit set, Unexpected end of message");
        }

        printf("%.*s", len, p);
        p += len;
        if (*p != 0)
        {
            printf("."); // www.naver.com 의 "."
            return print_name(msg, p, end);
        }
        else
        {
            return p + 1; // '3',com, 0 을 만난경우! 마지막 바이트를 읽었으므로 포인터+1
        }
    }
}

const void print_dns_message(const u_char *message, int length)
{
    if (length < 12) // 헤더길이는 12
    {
        errproc("print_name, length");
    }

    printf("Printing Header...\n");

    const u_char *msg = message;
    printf("ID = %0X %0X\n", msg[0], msg[1]); // 2바이트 ID

    const int qr = (msg[2] & 0x80) >> 7; // Query / Response Bit, 1000 0000 & xxxx xxxx -> 0000 000x
    printf("QR = %d %s\n", qr, qr ? "response" : "query");

    const int opcode = (msg[2] & 0x78) >> 3; // opcode(4 bit), xxxx xxxx & 0111 1000 -> 0000 xxxx
    printf("OPCODE = %d, ", opcode);
    switch (opcode)
    {
    case 0:
        printf("standard\n");
        break;
    case 1:
        printf("reverse\n");
        break;
    case 2:
        printf("status\n");
        break;
    default:
        printf("dunno\n");
        break;
    }

    const int aa = (msg[2] & 0x04) >> 2; // xxxx xXxx & 0000 0100 >> 2 (Authoritative Answer, 1-bit)
    printf("AA = %d %s\n", aa, aa ? "authoritative" : "");

    const int tc = (msg[2] & 0x02) >> 1; // xxxx xxXx & 0000 0010 >> 1 (Message Truncated, 1-bit)
    printf("TC = %d %s\n", tc, tc ? "truncated" : "");

    const int rd = (msg[2] & 0x01); // xxxx xxxX & 0000 0001 (Recursion Desired, 1-bit)
    printf("RD = %d %s\n", rd, rd ? "recursion desired" : "");

    if (qr)
    {
        const int ra = (msg[3] & 0x80); // Xxxx xxxx & 1000 0000 (Recursion Available, 1-bit)
        printf("RA = %d %s\n", ra, ra ? "recursion available" : "");

        const int rcode = (msg[3] & 0x0F); // xxxx XXXX & 0000 1111 (Response Code, 4-bit)
        printf("RCODE = %d ", rcode);
        switch (rcode)
        {
        case 0:
            printf("success\n");
            break;
        case 1:
            printf("format error\n");
            break;
        case 2:
            printf("server failure\n");
            break;
        case 3:
            printf("name error\n");
            break;
        case 4:
            printf("not implemented\n");
            break;
        case 5:
            printf("refused\n");
            break;

        default:
            printf("dunno\n");
            break;
        }
        if (rcode != 0)
            return;
    }

    const int qdcount = (msg[4] << 8) + msg[5]; // xxxx xxxx + 1000 0000 (Question Count, 16-bit)
    printf("qdcount = %d\n", qdcount);
    const int ancount = (msg[6] << 8) + msg[6]; // xxxx xxxx + 1000 0000 (Answer Count, 16-bit)
    printf("ancount = %d\n", ancount);
    const int nscount = (msg[8] << 8) + msg[7]; // xxxx xxxx + 1000 0000 (Name Server Count, 16-bit)
    printf("nscount = %d\n", nscount);
    const int arcount = (msg[10] << 8) + msg[11]; // xxxx xxxx + 1000 0000 (ARCOUNT, 16-bit)
    printf("arcount = %d\n", arcount);

    const u_char *p = msg + 12; // Post Header
    const u_char *end = msg + length;

    if (qdcount)
    {
        for (int i = 0; i < qdcount; ++i)
        {
            if (p >= end)
                errproc("qdcount, unexpected end of message");

            printf("Query %2d:\n", i + 1);
            printf("    name:");
            p = print_name(msg, p, end);
            printf("\n");

            if (p + 4 > end)
                errproc("unexpected end of message after name, before qtype\neither or both of them are not provided");
            // QType(2-bytes) + QClass(2-bytes) = 총 4-bytes. 이걸 넘어가면 문제있음

            const int qtype = (p[0] << 8) + p[1];
            printf("    qtype: %d\n", qtype);
            p += 2;

            const int qclass = (p[0] << 8) + p[1];
            printf("    qclass: %d\n", qclass);
            p += 2;
        }
    }

    if (ancount || nscount || arcount)
    {
        for (int i = 0; i < ancount + nscount + arcount; ++i)
        {
            if (p >= end)
                errproc("unexpected end of message while ancount iteration");

            printf("Answer %2d:\n", i + 1);
            printf("    name:");
            p = print_name(msg, p, end);
            printf("\n");

            if (p + 10 > end)
                errproc("unexpected end of message\nsome of formats are not provided");
            // TYPE =2 , CLASS=2, TTL=4, RDLENGTH=2 중 없는 거 있을 수 있다.

            const int type = (p[0] << 8) + p[1];
            printf("    type: %d\n", type);
            p += 2;

            const int class = (p[0] << 8) + p[1];
            printf("    class: %d\n", class);
            p += 2;

            const int ttl = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; // 4 bytes 읽기
            printf("    ttl: %d\n", ttl);
            p += 4;

            const int rdlen = (p[0] << 8) + p[1];
            printf("    rdlen: %d\n", rdlen);
            p += 2;

            printf("    rdata:\n");
            if (p + rdlen > end)
                errproc("rdlen, eom");

            if (rdlen == 4 && type == A)
            {
                printf("        Address: ");
                printf("%d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
            }
            else if (rdlen == 16 && type == AAAA)
            {
                printf("        Address: ");
                for (int j = 0; j < rdlen; j += 2)
                {
                    printf("%02x%02x\n", p[j], p[j + 1]);
                    if (j + 2 < rdlen)
                        printf(":");
                }
                printf("\n");
            }
            else if (type == MX && rdlen > 3)
            { /* MX Record */
                const int preference = (p[0] << 8) + p[1];
                printf(" pref: %d\n", preference);
                printf("MX: ");
                print_name(msg, p + 2, end);
                printf("\n");
            }
            else if (type == TXT)
            {
                /* TXT Record */
                printf("TXT: '%.*s'\n", rdlen - 1, p + 1);
            }
            else if (type == CNAME)
            {
                /* CNAME Record */
                printf("CNAME: ");
                print_name(msg, p, end);
                printf("\n");
            }

            p += rdlen;
        }
    }
}

int main(int argc, char **argv)
{
    
}