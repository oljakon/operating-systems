#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "info.h"

#define MAX_CLIENTS 10
int clients[MAX_CLIENTS] = { 0 };

void manageConnection(unsigned int fd)
{
    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr);

    // установка соединения в ответ на запрос клиента
    // функция accept() возвращает новый сокет, открытый
    // для обмена данными с клиентом, запросившим соединение
    
    int incom = accept(fd, (struct sockaddr*) &client_addr, (socklen_t*) &addrSize);
    if (incom < 0)
    {
        perror("Error in accept(): ");
        exit(-1);
    }

    printf("\nNew connection: \nfd = %d \nip = %s:%d\n", incom, 
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // сохраняем дескриптор в первый свободный
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == 0)
        {
            clients[i] = incom;
            printf("Managed as client #%d\n", i);
            break;
        }
    }

}

void manageClient(unsigned int fd, unsigned int client_id)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr);

    // сервер блокируется на функции recv() и ждет сообщения от процессов-клиентов
    
    int recvSize = recv(fd, msg, MSG_LEN, 0);
    if (recvSize == 0)
    {
        // возвращает адрес машины, подключившейся к сокету
        getpeername(fd, (struct sockaddr*) &client_addr, (socklen_t*) &addrSize);
        printf("User %d disconnected %s:%d \n", client_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        close(fd);
        clients[client_id] = 0;
    }
    else
    {
        msg[recvSize] = '\0';
        printf("Message from %d client: %s\n", client_id, msg);
    }
}

int main(void)
{
    // создание сетевого сокета (домен AF_INET,
    // тип сокета - SOCK_STREAM, сокет должен быть потоковым)
    // протокол - 0, выбирается по умолчанию
    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket < 0)
    {
        perror("Error in sock(): ");
        return my_socket;
    }

    struct sockaddr_in server_addr;
    // семейство адресов, которыми мы будем пользоваться
    server_addr.sin_family = AF_INET;
    // значение порта. Функция htons() переписываетзначение порта так, чтобы
    // порядок байтов соответствовал принятому в интернете
    server_addr.sin_port = htons(SOCK_PORT);
    // адрес (наша программа-сервер зарегистрируется на всех адресах машины,
    // на которой она выполняется)
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // связывание сокета с заданным адресом
    if (bind(my_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error in bind():");
        return -1;
    }
    printf("Server is listening on the %d port\n", SOCK_PORT);

    // переводим сервер в режим ожидания запроса на соединение
    // (второй параметр - максимальное количество соединений, обрабатываемых одновременно)
    if (listen(my_socket, 3) < 0)
    {
        perror("Error in listen(): ");
        return -1;
    }
    printf("Waiting for connections\n");

    for (;;)
    {
        fd_set readfds;
        int max_fd;
        int active_clients_count;

        FD_ZERO(&readfds);
        FD_SET(my_socket, &readfds);
        max_fd = my_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];

            if (fd > 0)
            {
                FD_SET(fd, &readfds);
            }

            max_fd = (fd > max_fd) ? (fd) : (max_fd);
        }
        
        // сервер блокируется на вызове функции select(), она возвращает управление,
        // если хотя бы один из проверяемых сокетов готов к выполнению соответствующей операции

        struct timeval timeout = {20, 0};
        active_clients = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (active_clients == 0)
        {
            printf("\nServer closed connection by timeout\n\n");
            return 0;
        }
        
        if (active_clients < 0 && (errno != EINTR))
        {
            perror("Error in select():");
            return active_clients_count;
        }

        // если есть новые соединения, вызывается функция manageConnection()
        if (FD_ISSET(my_socket, &readfds))
        {
            manageConnection(my_socket);
        }

        // обход массива дескрипторов
        // если дескриптор находится в наборе, запускается функция manageClient()
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if ((fd > 0) && FD_ISSET(fd, &readfds))
            {
                manageClient(fd, i);
            }
        }
    }

    return 0;
}
