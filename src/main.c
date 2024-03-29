#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "server.h"

#define MAX_BACKLOG 10
#define BUFF_SIZE 4096
#define PATH_SIZE 256
#define MAX_FILE_BYTES 2048
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

void resSendString(Request req, char *input);
void resSendFile(Request req, char *filepath);

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

void ServerRegister(server_t *app, char *path, HTTP_TYPE type, void (*handler)(Request, Response))
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

HTTP_TYPE str_to_http_type(char *input)
{
    HTTP_TYPE req_type = HTTP_NONE;
    // strcmp() returns 0 if matched
    if (!strcmp(input, "GET"))
        req_type = HTTP_GET;
    if (!strcmp(input, "POST"))
        req_type = HTTP_POST;
    return req_type;
}

void ServerRun(server_t *app)
{
    while (1)
    {
        /* -- Accept Connection -- */
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        // Accept is blocking until it gets a connection
        int client_fd = accept(app->_listenfd, (struct sockaddr *)&client_addr, &client_addr_size);
        ASSERT(client_fd != -1, "Failed to accept client!");

        char buffer[BUFF_SIZE] = {0};
        // TODO: Make sure bytes_read is correct
        int bytes_read = read(client_fd, buffer, BUFF_SIZE - 1);

        /* -- Read HTTP Type -- */
        // 10 characters just in case
        char first_chars[10] = {0};
        strncpy(first_chars, buffer, 4);
        first_chars[9] = '\0';

        // Probably not the best way to do this
        char *http_req_types[] = {"GET", "POST"};
        int http_req_nums = 2;

        regex_t regex;
        int result_found = false;
        HTTP_TYPE req_type = HTTP_NONE;
        for (int i = 0; i < http_req_nums; i++)
        {
            int comp_result = regcomp(&regex, http_req_types[i], 0);
            ASSERT(comp_result == 0, "Regex compilation failed!");
            result_found = regexec(&regex, first_chars, 0, NULL, 0);
            // regexec() returns 0 if matched
            if (result_found)
                continue;
            req_type = str_to_http_type(http_req_types[i]);
        }

        /* -- Read URL Path -- */
        // Again, probably not the best way to do this
        char path[PATH_SIZE] = {0};
        char *start_ptr = strchr(buffer, '/');
        int start_idx = start_ptr - buffer;
        {
            int i = 0;
            while (!isspace(buffer[start_idx + i]))
            {
                path[i] = buffer[start_idx + i];
                i++;
            }
            path[i] = '\0';
        }

        /* -- Route Request -- */
        if (req_type == HTTP_GET)
        {
            int found = false;
            Request requestObj;
            requestObj._client_fd = client_fd;
            strcpy(requestObj.path, path);

            Response responseObj;
            responseObj.sendFile = &resSendFile;
            responseObj.sendString = &resSendString;

            for (int i = 0; i < app->_get_count; i++)
            {
                if (strcmp(app->_get_handlers[i].path, path) == 0)
                {
                    app->_get_handlers[i].func(requestObj, responseObj);
                    found = true;
                    break;
                }
            }
            if (found == false)
            {
                app->config.not_found_handler(requestObj);
            }
        }
        if (req_type == HTTP_POST)
        {
            printf("POST REQUEST\n");
        }
    }
}

void resSendString(Request req, char *input)
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
    while (bytes_written != strlen(resp))
    {
        bytes_written = write(clientfd, resp, strlen(resp));
    }
    close(clientfd);
}

void resSendFile(Request req, char *filepath)
{
    int clientfd = req._client_fd;
    // TODO: Create http headers more dynamically
    char start[] = "HTTP/1.0 200 OK\r\n"
                   "Server: webserver-c\r\n"
                   "Content-type: text/html\r\n\r\n";
    char end[] = "\r\n";
    char file_contents[BUFF_SIZE];
    char buffer[MAX_FILE_BYTES];
    strcpy(file_contents, start);

    FILE *file = fopen(filepath, "r");
    ASSERT(file != NULL, "Failed to open file!");
    while (fgets(buffer, MAX_FILE_BYTES, file))
    {
        strcat(file_contents, buffer);
    }
    strcat(file_contents, end);

    int bytes_written = write(clientfd, file_contents, strlen(file_contents));
    while (bytes_written != strlen(file_contents))
    {
        bytes_written = write(clientfd, file_contents, strlen(file_contents));
    }
    shutdown(clientfd, SHUT_RDWR);
}