#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <time.h>

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

// Структура хранения статистики
typedef struct {
    int total_clients;
    int current_clients;
} Stats;

Stats stats = {0};

void set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
        perror("fcntl");
    else if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
        perror("fcntl non-blocking");
}

char* handle_command(const char* cmd) {
    static char response[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    
    // Обработка команд
    if (!strcmp(cmd, "/time")) {
        strftime(response, sizeof(response), "%Y-%m-%d %H:%M:%S\n", tm_now);
    }
    else if (!strcmp(cmd, "/stats")) {
        snprintf(response, sizeof(response),
                 "Total clients: %d\nCurrent connected: %d\n",
                 stats.total_clients,
                 stats.current_clients);
    }
    else if (!strcmp(cmd, "/shutdown")) {
        strcpy(response, "Shutting down...\n");
    }
    return response;
}

int main() {
    int tcp_sockfd, udp_sockfd, epollfd, nfds, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    struct epoll_event event, events[MAX_EVENTS];
    char buffer[BUFFER_SIZE], ipaddr[INET_ADDRSTRLEN];
    socklen_t addrlen;
    ssize_t nbytes;

    // Создаем сокеты
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(tcp_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(tcp_sockfd, 3);

    bind(udp_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // Настройка неблокирующего режима
    set_non_blocking(tcp_sockfd);
    set_non_blocking(udp_sockfd);

    // Создание epoll'а
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Регистрация TCP и UDP сокетов
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = tcp_sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, tcp_sockfd, &event);

    event.data.fd = udp_sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, udp_sockfd, &event);

    while (1) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == tcp_sockfd) {
                // Принять новое соединение
                connfd = accept(tcp_sockfd, NULL, NULL);
                if (connfd != -1) {
                    set_non_blocking(connfd);
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = connfd;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
                    printf("TCP client connected.\n");
                    stats.total_clients++;
                    stats.current_clients++;
                }
            }
            else if (events[i].data.fd == udp_sockfd) {
                // Получение пакетов по UDP
                addrlen = sizeof(cli_addr);
                nbytes = recvfrom(events[i].data.fd, buffer, BUFFER_SIZE, 0,
                                  (struct sockaddr*)&cli_addr, &addrlen);
                inet_ntop(AF_INET, &(cli_addr.sin_addr), ipaddr, INET_ADDRSTRLEN);
                printf("UDP packet from %s:%d (%ld bytes)\n", ipaddr, ntohs(cli_addr.sin_port), nbytes);
                
                if (buffer[0] == '/') {
                    const char* resp = handle_command(buffer + 1);
                    sendto(events[i].data.fd, resp, strlen(resp), 0,
                           (struct sockaddr*)&cli_addr, addrlen);
                } else {
                    sendto(events[i].data.fd, buffer, nbytes, 0,
                          (struct sockaddr*)&cli_addr, addrlen);
                }
            }
            else {
                // Чтение данных от клиента TCP
                nbytes = read(events[i].data.fd, buffer, BUFFER_SIZE);
                if (nbytes <= 0) {
                    close(events[i].data.fd);
                    stats.current_clients--;
                    continue;
                }

                if (buffer[0] == '/') {
                    const char* resp = handle_command(buffer + 1);
                    write(events[i].data.fd, resp, strlen(resp));
                } else {
                    write(events[i].data.fd, buffer, nbytes);
                }
            }
        }
    }

    return 0;
}
