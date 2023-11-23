// Server side C program to demonstrate Socket programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char const* argv[]) {
    int server_fd, new_socket;      // server_fd: server socket descriptor, new_socket: socket descriptor for communication with the client
    ssize_t valread;                // data type for the return value of read() function (number of bytes read)
    struct sockaddr_in address;     // socket address structure
    int opt = 1;                    // option used in setsockopt()
    socklen_t addrlen = sizeof(address);    // size of the socket address structure
    char buffer[1024] = { 0 };      // receive buffer
    char* hello = "Hello from server";      // message to be sent

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");    // error handling if socket creation fails
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");       // error handling if setting socket options fails
        exit(EXIT_FAILURE);
    }

    // Setting up the socket address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");       // error handling if binding socket and address fails
        exit(EXIT_FAILURE);
    }

    // Listening for client connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");           // error handling if setting up listening fails
        exit(EXIT_FAILURE);
    }

    // Accepting connection from the client
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("accept");           // error handling if accepting connection fails
        exit(EXIT_FAILURE);
    }

    // Receiving and displaying the message from the client
    valread = read(new_socket, buffer, 1024 - 1);
    printf("%s\n", buffer);

    // Sending a message from the server to the client
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // Closing the connected socket
    close(new_socket);
    // Closing the listening socket
    close(server_fd);

    return 0;
}