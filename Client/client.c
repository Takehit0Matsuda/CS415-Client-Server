// Client side C program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

//#define PORT 8080
#define BUFFER_SIZE 1024
#define END_FLAG '\n'

int main(int argc, char const* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    int status, valread, client_fd;    // status: return value of connect(), valread: return value of read(), client_fd: client socket descriptor
    struct sockaddr_in serv_addr;      // server address structure
    char* hello = "Hello from client"; // message to be sent
    char buffer[BUFFER_SIZE] = { 0 };         // receive buffer

    // Socket creation
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n"); // Error handling if socket creation fails
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n"); // Error handling if address conversion fails
        return -1;
    }

    // Establishing a connection to the server
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n"); // Error handling if connection fails
        return -1;
    }

    // Requesting the file name from the user
    printf("Enter file to retrieve: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strlen(buffer) - 1] = END_FLAG;  // Add the end-of-message flag
    // Remove the trailing newline character
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    printf("Connecting on Localhost at port %d to get file %s.\n", atoi(argv[1]), buffer);

    // Sending the file name to the server
    send(client_fd, buffer, strlen(buffer), 0);

    // Receive the file size from the client
    size_t file_size;
    ssize_t valread_size = read(client_fd, &file_size, sizeof(file_size));
    if (valread_size != sizeof(file_size)) {
        perror("Error reading file size");
        // Error handling
        return -1;
    }
    printf("File Size: %zu\n", file_size);

    // Receive data for the file size
    size_t total_received = 0;
    while (total_received < file_size) {
        ssize_t bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';  // Add null character to complete the C string
        printf("%.*s", (int)bytes_received, buffer);  // Display the received data
        total_received += bytes_received;
    }

    // Add a newline after END
    printf("\n");

    // Close the connected socket
    close(client_fd);

    return 0;
}