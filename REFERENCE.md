## Library Reference

A description of all functions and types available in the library

> Note: Some fields on structs or whole structs are undocumented and not shown; this is because you <b><i>should not</b></i> use them! These are internal only.

### Types

---

```c
// Request and Response are passed into handler callbacks
typedef struct
{
    char path[256]; // Path of request
} Request;

typedef struct
{
    void (*sendFile)(Request, char*); // Documented below in Functions
    void (*sendString)(Request, char*); // Documented below in Functions
} Response;

// Documented more thoroughly in CONFIG.md
struct server_config
{
    void (*not_found_handler)(Request);
};

typedef struct
{
    struct server_config config; // Config options struct
} server_t;

// Types of HTTP requests supported
typedef enum
{
    HTTP_GET,
    HTTP_POST
} HTTP_TYPE;
```

### Functions

---

```c

server_t CreateServer(struct server_config config);
void ServerListen(server_t *app, int port);
void ServerRegister(server_t *app, char *path, HTTP_TYPE type, void (*handler)(Request, Response));
void ServerRun(server_t *app);

/*
    Response.sendFile() takes in a Request object to find the client, and a string that represents the path to the file to send. Returns: void
    EX:
    Response.sendFile(req, "/page/index.html");
*/
Response.sendFile(Request, char*);
/*
    Response.sendString() takes in a Request object to find the client, and a string that represents the string to send to the client. Note: strings passed are prepended with <html> and appended with </html> automatically! Returns: void
    EX:
    Response.sendString(req, "Hello, world!");
*/
Response.sendString(Request, char*);
```
