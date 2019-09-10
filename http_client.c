#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 1000

char* options;
char* url;
char* port_num;

void send_GET(int client, char *url) {
    char buf[BUF_SIZE] = {0};
    sprintf(buf, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", url);
    send(client, buf, strlen(buf), 0);
}

int main(int argc, char** argv) {

    switch(argc) {
        case 3:
            url = argv[1];
            port_num = argv[2];
            break;
        case 4:
            options = argv[1];
            url = argv[2];
            port_num = argv[3];
            break;
        default:
            printf("Use: [-p] url portnumber\n");
            exit(1);
    }

    int status, sock, numbytes;
    struct addrinfo hints, *sinf;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //get address info, error checking
    if ((status = getaddrinfo(url, port_num, &hints, &sinf))) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    for (; sinf != NULL; sinf = sinf->ai_next) {
        //create socket
        if ((sock = socket(sinf->ai_family, sinf->ai_socktype, sinf->ai_protocol)) == -1) {
            perror("client: socket\n");
            continue;
        }
        //connect to socket, error checking
        if (connect(sock, sinf->ai_addr, sinf->ai_addrlen) == -1) {
            close(sock);
            perror("client: connect\n");
            continue;
        }

        break;
    }
    
    printf("client: connecting to %s\n", url);

    freeaddrinfo(sinf);

    send_GET(sock, url);

    char header[BUF_SIZE] = {'\0'};
    char search[] = "\r\n\r\n";

    for (int i = 0; i < BUF_SIZE; i++) {
        recv(sock, &header[i], 1, 0);
        if (strstr(header, search)) break;
    }
    printf("\n*********\nHEADER:\n*********\n\n%s", header);

    //int content_length = 1270;
    char *c_length;
    if ((c_length = strstr(header, "Content-Length: "))) {
        char *content_length = strstr(c_length, " ");
        int body_size;
        if (!(body_size = atoi(content_length))){
            fprintf(stderr, "Content length is 0. Exiting...\n");
            exit(1);
        }
        char body[body_size];

        int totalbytes = 0;
        // for (total_bytes = 0; total_bytes <= body_size; total_bytes += recv(sock, body, body_size, 0));
        //numbytes = recv(sock, body, body_size, 0);

        for (int i = 0; totalbytes < body_size; i++) {
            numbytes = recv(sock, &body[i], 1, 0);
            totalbytes += numbytes;
        }
        body[body_size] = '\0';
        printf("\n*********\nBODY:\n*********\n\n%s", body);

    } else if ((c_length = strstr(header, "Transfer-Encoding: "))) {
        int chunk_size = 0;
        printf("\n*********\nBODY:\n*********\n\n");
        do {
            char chunk_size_arr[10] = {'\0'};
            int i = 0;
            while (chunk_size_arr[i - 1] != '\n') {
                recv(sock, &chunk_size_arr[i], 1, 0);
                i++;
            }

            //include trailing \r\n unless chunk_size is 0
            if ((chunk_size = (int)strtol(chunk_size_arr, NULL, 16))) {
                chunk_size += 2;
            }

            char chunk[chunk_size];
            int totalbytes = 0, recv_size = chunk_size;
            i = 0;

            while(recv_size > 0) {
                numbytes = recv(sock, &chunk[i], recv_size, 0);
                totalbytes += numbytes;
                if (totalbytes <= chunk_size) recv_size -= numbytes;
                i += numbytes;         
            }
            chunk[chunk_size] = '\0';
            printf("%s", chunk);

        } while (chunk_size != 0);
    }

    printf("\n");
    close(sock);

    return 0;
}