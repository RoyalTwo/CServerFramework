#include <stdio.h>
#include "server.h"

void get_not_found(Request req)
{
    printf("Page not found!\n");
}

void get_root(Request req, Response res)
{
    printf("Root page!!\n");
    res.sendFile(req, "example/example.html");
}

void get_help(Request req, Response res)
{
    printf("Help page!!!\n");
}

int main(void)
{
    struct server_config config;
    config.not_found_handler = &get_not_found; // Required

    server_t app = CreateServer(config);
    ServerListen(&app, 3444);
    ServerRegister(&app, "/", HTTP_GET, &get_root);
    ServerRegister(&app, "/help", HTTP_GET, &get_help);

    ServerRun(&app);

    return 0;
}