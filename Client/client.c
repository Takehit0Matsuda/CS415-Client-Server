// Client side C program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char const* argv[]) {
    int status, valread, client_fd;    // status: return value of connect(), valread: return value of read(), client_fd: client socket descriptor
    struct sockaddr_in serv_addr;      // server address structure
    char* hello = "Hello from client"; // message to be sent
    char buffer[1024] = { 0 };         // receive buffer

    // Socket creation
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n"); // error handling if socket creation fails
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n"); // error handling if address conversion fails
        return -1;
    }

    // Establishing a connection to the server
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n"); // error handling if connection fails
        return -1;
    }

    // Sending a message from the client to the server
    send(client_fd, hello, strlen(hello), 0);
    printf("Hello message sent\n");

    // Receiving and displaying the message from the server
    valread = read(client_fd, buffer, 1024 - 1);
    printf("%s\n", buffer);

    // Closing the connected socket
    close(client_fd);

    return 0;
}