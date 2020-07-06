#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "info.h"

int main(void)
{
    // создание сокета (семейство адресов - AF_UNIX,
    // тип - SOCK_DGRAM - датаграммный сокет).
    // протокол - 0, выбирается по умолчанию
    
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Error in socket");
        return sockfd;
    }

    struct sockaddr server_addr;
    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, SOCKET_NAME);

    char msg[MSG_LEN]; 
    sprintf(msg, "Message from: %d\n", getpid());
    
    // передаем сообщение серверу
    // параметры sendto - дескриптор сокета, адрес буфера для передачи
    // данных, длина буфера, дополнительные флаги, адрес сервера, его длина
    
    sendto(sockfd, msg, strlen(msg), 0, &server_addr, sizeof(server_addr));

    close(sockfd);
    return 0;
}
