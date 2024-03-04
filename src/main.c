#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "main.h"

#define PORT 3444
#define MAX_BACKLOG 10
#define BUFF_SIZE 4096
#define true 1
#define false 0

void ASSERT(int condition, char *message)
{
    if (!condition)
    {
        printf("ASSERTION FAILED: %s\n", message);
        abort();
    }
}

server_t CreateServer(struct server_config conf)
{
    server_t srv = {0};
    srv.config = conf;
    return srv;
}

void ServerListen(server_t *app, int port)
{
    int server_fd;
    struct sockaddr_in server_addr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(server_fd != -1, "Failed to create socket!");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int enable = 1;
    int setopt_ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    ASSERT(setopt_ret != -1, "Failed to set socket options!");

    int bind_ret = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    ASSERT(bind_ret != -1, "Failed to bind socket!");

    // TODO: Change MAX_BACKLOG
    int listen_ret = listen(server_fd, MAX_BACKLOG);
    ASSERT(listen_ret != -1, "Failed to listen on socket!");

    app->_listenfd = server_fd;
    app->_addr = server_addr;
}

void ServerRegister(server_t *app, char *path, HTTP_TYPE type, void (*handler)(Request))
{
    if (type == HTTP_GET)
    {
        struct get_handler g_handler;
        g_handler.func = handler;
        strcpy(g_handler.path, path);
        app->_get_handlers[app->_get_count] = g_handler;
        app->_get_count++;
    }
}

void ServerRun(server_t *app)
{
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        // Accept is blocking until it gets a connection
        int client_fd = accept(app->_listenfd, (struct sockaddr *)&client_addr, &client_addr_size);
        ASSERT(client_fd != -1, "Failed to accept client!");

        char buffer[BUFF_SIZE] = {0};
        int bytes_read = read(client_fd, buffer, BUFF_SIZE - 1);
        // 4 for GET, POST, HEAD
        char req[5];
        req[4] = '\0';
        strncpy(req, buffer, 4);
        // TODO: change from 256
        char path[256] = {0};
        // This is horrible spaghetti code, I should use regex
        // TODO: improve please :C
        for (int i = 0; i < 256; i++)
        {
            if (isspace(buffer[i + 4]) != 0)
            {
                path[i] = '\0';
                break;
            }
            path[i] = buffer[i + 4];
        }

        if (strcmp(req, "GET ") == 0)
        {
            int found = false;
            Request requestObj;
            requestObj._client_fd = client_fd;
            strcpy(requestObj.path, path);

            for (int i = 0; i < app->_get_count; i++)
            {
                if (strcmp(app->_get_handlers[i].path, path) == 0)
                {
                    app->_get_handlers[i].func(requestObj);
                    found = true;
                    break;
                }
            }
            if (found == false)
            {
                app->config.not_found_handler(requestObj);
            }
        }
        if (strcmp(req, "POST") == 0)
        {
            printf("POST REQUEST\n");
        }
    }
}

void ResSendString(Request req, char *input)
{
    int clientfd = req._client_fd;
    // TODO: change from 1024
    char resp[1024] = "HTTP/1.0 200 OK\r\n"
                      "Server: webserver-c\r\n"
                      "Content-type: text/html\r\n\r\n"
                      "<html>";
    char end[] = "</html>\r\n";
    strcat(resp, input);
    strcat(resp, end);

    int bytes_written = write(clientfd, resp, strlen(resp));
    close(clientfd);
}

void ResSendFile(Request req, char *filepath)
{
    int clientfd = req._client_fd;
    char start[] = "HTTP/1.0 200 OK\r\n"
                   "Server: webserver-c\r\n"
                   "Content-type: text/html\r\n\r\n";
    char end[] = "\r\n";
    // TODO: change from 4096
    char file_contents[4096];
    char buffer[1024];
    strcpy(file_contents, start);

    FILE *file = fopen(filepath, "r");
    ASSERT(file != NULL, "Failed to open file!");
    while (fgets(buffer, 1024, file))
    {
        strcat(file_contents, buffer);
    }
    strcat(file_contents, end);

    write(clientfd, file_contents, strlen(file_contents));
    shutdown(clientfd, SHUT_RDWR);
}