/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to
#define BUF_SIZE 1000

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            int size;
            char buf[BUF_SIZE];
            char header[BUF_SIZE];
            char *method;
            char *filename;
            int content_length;
            FILE *fp;

            recv(new_fd, buf, BUF_SIZE, 0);

            printf("buf: %s", buf);

            method = strtok(buf, " ");
            //if (strcmp(method, "GET") != 0) return;
            filename = strtok(NULL, " ");
            if (filename[0] == '/') filename++;
            
            if (access(filename, F_OK) != 0) {
                sprintf(header, "HTTP/1.1 404 Not Found\r\n\r\n");
            } else {
                fp = fopen(filename, "r");
                for (content_length = 0; getc(fp) != EOF; content_length++);
                fclose(fp);
                fp = fopen(filename, "r");
                sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", content_length);
            }

            if ((send(new_fd, header, strlen(header), 0)) == -1) {
                perror("send header");
            }
            //filename = "files/TMDG.html";//strtok(NULL, " ");
            
            
            while (fgets(buf, BUF_SIZE, fp)) {
                if ((send(new_fd, buf, strlen(buf), 0)) == -1) {
                    perror("send file");
                }
                memset(buf, 0, BUF_SIZE);
            }

        //     FILE *fp = fopen("starter/Project_1_A19/TMDG.html", "r");
        //     // int content_length;
        //     // for (content_length = 0; getc(fp) != EOF; content_length++);
        //     // printf("File Size: %d\n", content_length);
        //     int content_length = 58327;





        //     char header[1000];
        //     sprintf(header, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", content_length);
        //     char buf[1000];


        //     // char header = ("HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", content_length);
        //     // int header_length = strlen(header);
        //     // char buf[header_length + content_length] = {'\0'};
        //     // char content_buf = buf + header_length;

            


        //     //while(fgets(buf, 1000, header)) {
        //         if ((send(new_fd, header, strlen(header), 0)) == -1) {
        //             perror("send");
        //         }
        //         //memset(buf, 0, 1000);
        //     //}



        //     while(fgets(buf, 1000, fp)) {
        //         if ((send(new_fd, buf, strlen(buf), 0)) == -1) {
        //             perror("sendasdfas");
        //         }
        //         memset(buf, 0, 1000);
        //     }



            
        //     // if (send(new_fd, "Hello, world!", 13, 0) == -1)
        //     //     perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
    close(sockfd);
    return 0;
}