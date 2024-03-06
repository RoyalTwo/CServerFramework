## C Server Framework

### What?

This is a framework for C to enable easily creating servers in a style reminiscent of ExpressJS for Node and JavaScript.

### Why?

I enjoy using C and wanted to try using C as a server backend, but found that dealing with sockets manually is extremely tedious.
This framework aims to alleviate some of the problems I faced and allow for a more modular way to create servers.

-   Note: this framework is <b><i>not</b></i> production-ready. It's a hobby project with many missing features, vulnerabilities, etc.

### Building

> If you'd prefer to just download the library files, download libserver.a and server.h in /bin/

Requirements: GNU make, clang

1. Clone the repository to a folder of your choosing
2. cd into your chosen folder
3. Run `make build` to build the newest version (output is in /bin/)

-   _Optional: Run `make run-example` to run the example code found in /example/_

#### Example Build

```
$ git clone https://github.com/RoyalTwo/CServerFramework.git
$ cd CServerFramework/
$ make build
```

Go to <b>Usage</b> for a reference on how to use the library.

### Installation

To allow the library to be used:

1. Copy libserver.a (either from building or downloading) to a directory in your project
2. Copy server.h to your include directory
3. When building your project, remember to set the include path and add libserver.a before your C files
    - Ex: `clang libserver.a main.c -Ilib/ -o main.out`

### Usage

This library is designed to be used in a similar way to ExpressJS. That means HTTP requests are handled by registering and passing a handler function callback. These callback functions take in two arguments, a Request object and a Response object.

Below is an example of a simple web server that handles a GET request to the root domain. There is also an example/ folder that showcases every feature.

```c
#include <stdio.h>
#include "server.h" // Include the library

#define PORT 8080 // Port can be any valid port

void get_not_found(Request req)
{
    // This config function only takes a Request object
    printf("Requested a page not found!");
}

void get_root(Request req, Response res)
{
    // This is an example handler function.
    // Here we'll print to the server that someone is on the root page, and send the client the string "Hello, world!" as HTML.
    printf("Visited the root page!");
    res.sendString("Hello, world!");
}

int main(void)
{
    struct server_config config; // struct containing config options. See config.md for options
    config.not_found_handler = &get_not_found; // Set not_found handler. REQUIRED!

    server_t app = CreateServer(config);
    ServerListen(&app, PORT); // Tell the server what port to listen on
    /*
     ServerRegister() takes in the server, the URL path to register this function on, the type of request to handle, and the callback function to handle it.
     The following line registers the get_root() function as the handler for a GET request at the root url '/'.
    */
    ServerRegister(&app, "/", HTTP_GET, &get_root);

    // Start the server
    ServerRun(&app);

    return 0;
}

```

### Config

See `CONFIG.md`

### Reference

See `REFERENCE.md`

---

<p align="center"> Made with ❤️ by RoyalTwo! </p>

---
