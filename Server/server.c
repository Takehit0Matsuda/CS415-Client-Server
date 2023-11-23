// Server side C program to demonstrate Socket programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>  // Added: Header file for threads
#include <limits.h>

//#define PORT 8080
#define BUFFER_SIZE 1024
#define END_FLAG '\n'  // Added: Flag indicating the end of a message

// Added: Function for threads
void *handle_request(void *socket_desc) {
    int new_socket = *(int *)socket_desc;
    ssize_t valread;
    char buffer[BUFFER_SIZE] = {0};

    // Receiving the file name from the client
    valread = read(new_socket, buffer, BUFFER_SIZE - 1);
    buffer[valread] = '\0';  // Add null character to complete the C string
    printf("From Client (%zd): %.*s\n", valread, (int)valread, buffer);
    // Build the file path
    char file_path[PATH_MAX];  // PATH_MAX is the standard definition for the maximum path length
    snprintf(file_path, sizeof(file_path), "root/%s", buffer);

    // TODO: Open the file, get its size, and send the size to the client
    printf("Opening root/%s\n", buffer);
    FILE *file = fopen(file_path, "rb");  // Open the file in binary mode

    if (file == NULL) {
        perror("Error opening file");
        // Additional processing, such as sending an error message to the client, could be added here
    } else {
        fseek(file, 0, SEEK_END);  // Move to the end of the file
        size_t file_size = ftell(file);  // Get the file size
        printf("File is %zd bytes\n", file_size);
        fseek(file, 0, SEEK_SET);  // Move to the beginning of the file

        // Send the file size to the client
        printf("Sending %zd\n", file_size);
        send(new_socket, &file_size, sizeof(file_size), 0);

        // Display the file contents
        size_t total_sent = 0;
        size_t bytes_read;

        while (total_sent < file_size) {
            bytes_read = fread(buffer, 1, sizeof(buffer), file);
            printf("%.*s", (int)bytes_read, buffer);  // Display the received data
            send(new_socket, buffer, bytes_read, 0);
            total_sent += bytes_read;
        }

        fclose(file);  // Close the file
    }

    // Notify the client that the processing has finished
    send(new_socket, "END\n", strlen("END\n"), 0);

    // Close the connected socket
    close(new_socket);

    // Exit the thread
    pthread_exit(NULL);
}

int main(int argc, char const* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket;      // server_fd: server socket descriptor, new_socket: socket descriptor for communication with the client
    ssize_t valread;                // data type for the return value of read() function (number of bytes read)
    struct sockaddr_in address;     // socket address structure
    int opt = 1;                    // option used in setsockopt()
    socklen_t addrlen = sizeof(address);    // size of the socket address structure
    char buffer[BUFFER_SIZE] = { 0 };      // receive buffer
    char* hello = "Hello from server";      // message to be sent

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");    // Error handling if socket creation fails
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");       // Error handling if setting socket options fails
        exit(EXIT_FAILURE);
    }

    // Setting up the socket address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    printf("Launching Server on Port: %d\n", atoi(argv[1]));

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");       // Error handling if binding socket and address fails
        exit(EXIT_FAILURE);
    }

    // Listening for client connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");           // Error handling if setting up listening fails
        exit(EXIT_FAILURE);
    }

    // Accepting connections and launching threads for each request
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

        // Detach the thread to clean up resources automatically
        pthread_detach(thread_id);
        
        printf("\n");
    }

    // Not reached in this example, but closing the listening socket
    close(server_fd);

    return 0;
}