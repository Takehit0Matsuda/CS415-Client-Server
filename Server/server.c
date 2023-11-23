// Server side C program to demonstrate Socket programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>

#define BUFFER_SIZE 1024
#define END_FLAG '$'

void *handle_request(void *socket_desc) {
    int new_socket = *(int *)socket_desc;
    ssize_t valread;
    char buffer[BUFFER_SIZE] = {0};

    // Receiving the file name from the client
    valread = read(new_socket, buffer, BUFFER_SIZE - 1);
    buffer[valread] = '\0';  // Add null character to complete the C string

    // Remove the end-of-message flag from the file name
    char *flag_position = strchr(buffer, END_FLAG);
    if (flag_position != NULL) {
        *flag_position = '\0';
    }

    printf("From Client (%zd): %s\n", valread, buffer);

    char file_path[PATH_MAX];
    snprintf(file_path, sizeof(file_path), "root/%s", buffer);

    printf("Opening root/%s\n", buffer);
    FILE *file = fopen(file_path, "rb");

    if (file == NULL) {
        perror("Error opening file");
        send(new_socket, "ERROR\n", strlen("ERROR\n"), 0); // Inform client about the error
    } else {
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        printf("File is %zd bytes\n", file_size);
        fseek(file, 0, SEEK_SET);

        // Send the file size to the client with the end flag
        snprintf(buffer, sizeof(buffer), "%zu%c", file_size, END_FLAG);
        printf("Sending: %s\n", buffer);
        send(new_socket, buffer, strlen(buffer), 0);

        size_t total_sent = 0;
        size_t bytes_read;

        while (total_sent < file_size) {
            bytes_read = fread(buffer, 1, sizeof(buffer), file);
            printf("%.*s", (int)bytes_read, buffer);
            send(new_socket, buffer, bytes_read, 0);
            total_sent += bytes_read;
        }

        fclose(file);
    }

    // Notify the client that the processing has finished
    send(new_socket, END_FLAG, 1, 0);

    // Close the connected socket
    close(new_socket);

    // Exit the thread
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *hello = "Hello from server";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    printf("Launching Server on Port: %d\n", atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_request, (void *)&new_socket) < 0) {
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }

        pthread_detach(thread_id);

        printf("\n");
    }

    close(server_fd);

    return 0;
}