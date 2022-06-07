#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#define TIMEOUT 5.0

void parse_url(char *url, char **hostname, char **port, char **path)
{
    printf("URL: %s\n", url);

    char *p;
    p = strstr(url, "://");

    char *protocol = 0;
    if (p)
    {
        protocol = url; // *http://example.com
        *p = 0;         // http*:// -> http'\0'//
        p += 3;         // *exmaple.com
    }
    else
    {
        p = url; // Protocol 지정 안된 경우
    }

    if (protocol)
    {
        if (strcmp(protocol, "http"))
        {
            fprintf(stderr, "Protocol not available: %s\n", protocol);
            exit(1);
        }
    }

    *hostname = p;
    while (*p && *p != ':' && *p != '/' && *p != '#')
        ++p;

    if (*p == ':')
    {
        *p++ = 0; // example.com:80 -> ple.com'\0'80
        *port = p;
    }
    else
    {
        *port = "80"; // on case: exmaple.com/sadf
    }
    while (*p && *p != '/' && *p != '#')
        ++p;

    *path = p;
    if (*p == '/')
    {
        *path = p + 1;
    }
    *p++ = 0; // exmaple.com -> example.com\0 example.com/asdf -> example.com'\0'asdf

    while (*p && *p != '#')
        ++p;
    if (*p == '#')
        *p = 0;

    printf("protocol: %s\nhostname: %s\nport: %s\npath: %s\n", protocol, *hostname, *port, *path);
}

void send_request(SOCKET s, char *hostname, char *port, char *path)
{
    char buff[2048];

    sprintf(buff, "GET /%s HTTP/1.1\r\n", path);
    sprintf(buff + strlen(buff), "Host: %s:%s\r\n", hostname, port);
    sprintf(buff + strlen(buff), "Connection: close\r\n");
    sprintf(buff + strlen(buff), "User-Agent: web_get test\r\n");
    sprintf(buff + strlen(buff), "\r\n");

    send(s, buff, strlen(buff), 0);
    printf("Sent Headerss: \n%s", buff);
}

SOCKET connect_to_host(char *hostname, char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if(getaddrinfo(hostname, port, &hints, &peer_address))
    {
        fprintf(stderr, "ERROR GETADDRINFO\n");
        exit(1);
    }

    printf("remote address resolved: ");
    char address_buffer[100], service_buffer[100]; // address, port
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    printf("Creating Socket...\n");
    SOCKET server;
    server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(server))
    {
        fprintf(stderr, "ERROR SOCKET\n");
        exit(1);
    }

    printf("Connecting...\n");
    if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "ERROR CONNECT\n");
        exit(1);
    }

    freeaddrinfo(peer_address);
    printf("Connected\n\n");
    return server;
}

int main(int argc, char **argv)
{
    char *url = argv[1];
    char *hostname, *port, *path;
    parse_url(url, &hostname, &port, &path);

    SOCKET connect_socket = connect_to_host(hostname, port);
    send_request(connect_socket, hostname, port, path);

    const clock_t start_time = clock();

#define RESPONSE_SIZE 32768
    char response[RESPONSE_SIZE];
    char *p = response, *q;
    char *end = response + RESPONSE_SIZE;
    char *body = 0;

    enum
    {
        length,
        chunked,
        connection
    };
    int encoding = 0;
    int remaining = 0;

    while (1)
    {
        if ((clock() - start_time) / CLOCKS_PER_SEC > TIMEOUT)
        {
            fprintf(stderr, "TIMEOUT after %.2f seconds\n", TIMEOUT);
            return 1;
        }

        if (p == end)
        {
            fprintf(stderr, "OUT OF BUFFER SPACE\n");
            return 1;
        }

        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(connect_socket, &reads);

        if (select(connect_socket + 1, &reads, 0, 0, 0) < 0)
        {
            fprintf(stderr, "SELECT FAILED %d\n", GETSOCKETERRNO());
            return 1;
        }
        if (FD_ISSET(connect_socket, &reads))
        {
            int bytes_received = recv(connect_socket, p, end - p, 0);
            if (bytes_received < 1)
            {
                if (encoding == connection && body)
                {
                    printf("%.*s", (int)(end - body), body);
                }
                printf("Connection Closed by peer\n");
                break;
            }

            p += bytes_received; // 끝으로
            *p = 0;              // NULL-TERMINATED STRING

            if (!body && (body = strstr(response, "\r\n\r\n")))
            {
                *body = 0; //헤더 끝에 NULL
                body += 4; // BODY 시작으로

                printf("Received Headers: \n%s\n", response);
            }
            q = strstr(response, "\nContent-Length: ");
            if (q)
            {
                encoding = length;
                q = strchr(q, ' ');
                q += 1;
                remaining = strtol(q, 0, 10); // Content-Length 를 Remaining으로, remaining이 바디 길이
            }
            else
            {
                q = strstr(response, "\nTransfer-Encoding: chunked");
                if (q)
                {
                    encoding = chunked;
                    remaining = 0;
                }
                else
                {
                    encoding = connection;
                }
            }

            printf("Received body: \n");
            if (body)
            {
                if (encoding == length)
                {
                    if (p - body >= remaining) // 끝 - 시작 >= 받은 바디 길이보다 크면
                    {
                        printf("%.*s", remaining, body);
                        break;
                    }
                }
                else if (encoding == chunked)
                {
                    do
                    {
                        if (remaining == 0)
                        {
                            if ((q = strstr(body, "\r\n"))) // 다 읽기 전까진 \r\n이 있음.
                            {
                                remaining = strtol(body, 0, 16);
                                if (!remaining)
                                    goto finish;
                                body = q + 2;
                            }
                            else
                            {
                                break;
                            }
                        }
                        if (remaining && p - body >= remaining)
                        {
                            printf("%.*s", remaining, body);
                            body += remaining + 2; // \r\n 스킵
                            remaining = 0;
                        }
                    } while(!remaining);
                }
            }
        }
    }

finish:
    return 0;
}