// Client side C program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define END_FLAG '$'

int main(int argc, char const *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("\nConnection Failed \n");
        return -1;
    }

    printf("Enter file to retrieve: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strlen(buffer) - 1] = END_FLAG; // Add the end-of-message flag
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    printf("Connecting on Localhost at port %d to get file %s.\n", atoi(argv[1]), buffer);

    send(client_fd, buffer, strlen(buffer), 0);

    // Receive the file size from the server
    size_t file_size;
    char size_buffer[BUFFER_SIZE];
    ssize_t valread_size = 0;
    while (1) {
        ssize_t bytes_received = read(client_fd, size_buffer + valread_size, 1);
        if (bytes_received <= 0) {
            break;
        }
        valread_size += bytes_received;
        if (size_buffer[valread_size - 1] == END_FLAG) {
            break;
        }
    }

    // Convert the received size to a size_t
    size_buffer[valread_size - 1] = '\0';
    file_size = atoi(size_buffer);

    printf("File Size: %zu\n", file_size);

    // Receive data for the file size
    size_t total_received = 0;
    while (total_received < file_size) {
        ssize_t bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0'; // Add null character to complete the C string
        printf("%s", buffer);         // Display the received data
        total_received += bytes_received;
            
        if (strchr(buffer, END_FLAG) != NULL) {
            total_received--;
            break;
        }
    }

    printf("\n");

    close(client_fd);

    return 0;
}