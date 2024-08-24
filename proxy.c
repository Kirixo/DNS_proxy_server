//
// Created by kirixo on 23.08.24.
//
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 4096

char* blocked_domains[] = {
    "rozetka.com",
    "youtube.com",
    NULL
};



int is_blocked(const char* domain) {
    for(int i = 0; blocked_domains[i] != NULL; ++i) {
        if(!strcmp(domain, blocked_domains[i])) {
            return 1;
        }
    }
    return 0;
}

void start_proxy() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(listen(server_fd, 3) , 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Proxy server started on port %d\n", PORT);

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread < 0) {
            perror("read failed");
            close(new_socket);
            continue;
        }

        char *host = strstr(buffer, "Host: ");
        if (host) {
            host += 6;
            char *end = strstr(host, "\r\n");
            if (end) {
                *end = '\0';
            }

            printf("Request for host: %s\n", host);

            char *redirect_ip = "172.161.138.138";

            if (is_blocked(host)) {
                printf("blocked host: %s", host);
                char redirect_msg[BUFFER_SIZE];
                snprintf(redirect_msg, sizeof(redirect_msg),
                         "HTTP/1.1 302 Found\r\n"
                         "Location: http://%s/\r\n"
                         "Content-Length: 0\r\n"
                         "\r\n",
                         redirect_ip);
                write(new_socket, redirect_msg, strlen(redirect_msg));
                printf("Redirected to: %s\n", redirect_ip);
            } else {
                const char *ok_msg = "HTTP/1.1 200 OK\r\n\r\nProxy passed request\r\n";
                write(new_socket, ok_msg, strlen(ok_msg));
                printf("Allowed: %s\n", host);
            }
        } else {
            printf("No host found in request.\n");
        }

        close(new_socket);
    }
}

int main() {
    start_proxy();
    return 0;
}