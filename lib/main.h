#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct
{
    char path[256];
    int _client_fd;
} Request;

typedef struct
{
    void (*sendFile)(Request, char *);
    void (*sendString)(Request, char *);
} Response;

// Response should have sendFile() and sendString()

struct get_handler
{
    void (*func)(Request, Response);
    // TODO: change from 256
    char path[256];
};

struct server_config
{
    void (*not_found_handler)(Request);
};

typedef struct
{
    struct server_config config;
    int _listenfd;
    struct sockaddr_in _addr;
    struct get_handler _get_handlers[256];
    int _get_count;
} server_t;

typedef enum
{
    HTTP_GET,
    HTTP_POST
} HTTP_TYPE;

server_t CreateServer(struct server_config conf);
void ServerListen(server_t *app, int port);
void ServerRegister(server_t *app, char *path, HTTP_TYPE type, void (*handler)(Request, Response));
void ServerRun(server_t *app);

void ResSendString(Request req, char *input);
void ResSendFile(Request req, char *filepath);