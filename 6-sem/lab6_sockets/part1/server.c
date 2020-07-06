#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h> //socket, bind

#include "info.h"

int sockfd;

int main(void)
{
    char msg[MSG_LEN]; 
    struct sockaddr client_addr;
    
// создание сокета (семейство адресов – AF_UNIX, тип – SOCK_DGRAM – датаграммный сокет).
// протокол – 0, выбирается по умолчанию
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error in socket");
        return sockfd;
    }

    // семейство адресов, которым мы будем пользоваться
    client_addr.sa_family = AF_UNIX;
    // имя файла сокета
    strcpy(client_addr.sa_data, SOCKET_NAME);
    
// связывание сокета с заданным адресом
// параметры bind – дескриптор сокета, указатель на структуру, длина структуры
    if (bind(sockfd, &client_addr, sizeof(client_addr)) < 0)
    {
        printf("Can'tbind name to socket\n");
        close(sockfd);
        unlink(SOCKET_NAME);
        perror("Error in bind(): ");
        return -1;
    }
    
    // программа-сервер становится доступна для соединения по заданному адресу (имени файла)

    printf("\nServer is waiting for the message...\n");
    
    // сервер блокируется на функции recv() и ждет сообщения от процессов-клиентов

    for(;;)
    {
        int recievedSize = recv(sockfd, msg, sizeof(msg), 0);
        if (recievedSize < 0) 
        {
            close(sockfd);
            unlink(SOCKET_NAME);
            perror("Error in recv(): ");
            return recievedSize;
        }

        msg[recievedSize] = 0;
        printf("Client send message: %s\n", msg);
    }
    
    printf("Closing socket\n");
    close(sockfd);
    unlink(SOCKET_NAME);
    return 0;
}
