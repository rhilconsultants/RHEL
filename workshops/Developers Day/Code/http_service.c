#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <arpa/inet.h>  // For socket functions, inet_ntoa
#include <sys/socket.h> // For socket functions
#include <netdb.h>      // For gethostname()
#include <pthread.h>    // For pthreads

#define PORT 8080
#define BUFFER_SIZE 4096 // Max size for HTTP request/response
#define MAX_JSON_VALUE_LEN 256 // Max length for extracted JSON string values

// Global variable for hostname, as it's constant for all threads
static char global_hostname[256];

/**
 * @brief Extracts a string value associated with a given key from a JSON string.
 * This is a very simplistic JSON parser. It assumes the key-value pair
 * is flat and well-formed within the input string.
 * @param json_str The input JSON string.
 * @param key The key to search for (e.g., "name", "sentence").
 * @param value_buffer Buffer to store the extracted value.
 * @param buffer_size Size of the value_buffer.
 * @return 0 on success, -1 if key not found or buffer too small.
 */
int get_json_string_value(const char *json_str, const char *key, char *value_buffer, size_t buffer_size) {
    char search_key[128];
    // Form the search key for JSON parsing (e.g., "\"sentence\": \"")
    snprintf(search_key, sizeof(search_key), "\"%s\": \"", key);

    // Find the start of the value
    char *start = strstr(json_str, search_key);
    if (!start) {
        return -1; // Key not found
    }

    start += strlen(search_key); // Move past the key and the opening quote of the value

    // Find the end of the value (closing quote)
    char *end = strchr(start, '"');
    if (!end) {
        return -1; // End quote not found
    }

    // Calculate the length of the extracted value
    size_t len = end - start;
    if (len >= buffer_size) {
        return -1; // Buffer too small to hold the value
    }

    // Copy the value into the buffer and null-terminate it
    strncpy(value_buffer, start, len);
    value_buffer[len] = '\0';

    return 0;
}

/**
 * @brief Function to handle individual client connections in a separate thread.
 * This function encapsulates the logic for processing a single HTTP request.
 * @param socket_desc_ptr A pointer to the client socket descriptor (int).
 * @return NULL on completion.
 */
void *handle_client(void *socket_desc_ptr) {
    // Cast the void pointer back to an integer pointer to get the socket descriptor
    int new_socket = *(int *)socket_desc_ptr;
    // Free the dynamically allocated memory for the socket descriptor
    // that was passed from the main thread.
    free(socket_desc_ptr);

    char buffer[BUFFER_SIZE] = {0};
    char client_ip[INET_ADDRSTRLEN];
    int client_port;

    // Get client information (optional, but good for logging)
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    if (getpeername(new_socket, (struct sockaddr*)&client_addr, &client_addr_len) == 0) {
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        client_port = ntohs(client_addr.sin_port);
        printf("Thread handling connection from %s:%d (socket %d)\n", client_ip, client_port, new_socket);
    } else {
        perror("getpeername failed");
        printf("Thread handling connection from unknown client (socket %d)\n", new_socket);
    }


    // Read the incoming request into the buffer
    ssize_t bytes_received = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == -1) {
        perror("recv failed in thread");
        close(new_socket);
        return NULL; // Exit thread
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data

    printf("Received request on socket %d:\n%s\n", new_socket, buffer);

    // --- Basic HTTP and JSON parsing (same as before) ---
    char *post_method = strstr(buffer, "POST ");
    char *content_length_header = strstr(buffer, "Content-Length: ");
    char *http_body = NULL;
    size_t content_length = 0;

    if (post_method && content_length_header) {
        // Extract Content-Length value
        sscanf(content_length_header, "Content-Length: %zu", &content_length);

        // Find the start of the HTTP body (after the double CRLF)
        http_body = strstr(buffer, "\r\n\r\n");
        if (http_body) {
            http_body += 4; // Move past "\r\n\r\n"
        }
    }

    char received_sentence[MAX_JSON_VALUE_LEN] = {0};

    if (http_body && content_length > 0) {
        // Attempt to parse the 'sentence' from the JSON body
        if (get_json_string_value(http_body, "sentence", received_sentence, sizeof(received_sentence)) == 0) {
            printf("Extracted sentence: '%s' from socket %d\n", received_sentence, new_socket);
        } else {
            printf("Could not extract 'sentence' from JSON or JSON invalid on socket %d.\n", new_socket);
        }
    } else {
        printf("Not a valid POST request with Content-Length or no body on socket %d.\n", new_socket);
    }

    // --- Prepare the JSON and HTTP response (same as before) ---
    char json_response[BUFFER_SIZE];
    // Use the extracted sentence if available, otherwise a placeholder
    const char *sentence_to_send = (strlen(received_sentence) > 0) ? received_sentence : "No sentence received.";

    // Use the global hostname
    snprintf(json_response, sizeof(json_response),
             "{\"hostname\": \"%s\", \"sentence\": \"%s\"}",
             global_hostname, sentence_to_send);

    char http_response[BUFFER_SIZE];
    size_t json_len = strlen(json_response);

    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n" // Indicate that the connection will be closed after this response
             "\r\n"
             "%s",
             json_len, json_response);

    // Send the HTTP response
    if (send(new_socket, http_response, strlen(http_response), 0) == -1) {
        perror("send failed in thread");
    } else {
        printf("Response sent on socket %d:\n%s\n", new_socket, http_response);
    }

    // Close the client socket for this thread
    close(new_socket);
    printf("Connection on socket %d closed. Thread exiting.\n\n", new_socket);

    return NULL; // Indicate successful thread execution
}

/**
 * @brief Main function to run the HTTP service.
 * This main function will now create new threads for each client connection.
 */
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id; // To store the ID of the new thread

    // Get the server hostname once at startup
    if (gethostname(global_hostname, sizeof(global_hostname)) == -1) {
        perror("gethostname failed");
        exit(EXIT_FAILURE);
    }
    global_hostname[sizeof(global_hostname) - 1] = '\0'; // Ensure null-termination

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available network interfaces
    address.sin_port = htons(PORT);       // Convert port number to network byte order

    // Bind the socket to the specified IP address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) == -1) { // 10 is the backlog queue size
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d. Hostname: %s\n", PORT, global_hostname);
    printf("Ready to accept connections...\n");

    while (1) {
        // Accept a new connection
        // This call blocks until a client connects.
        // Once accepted, `new_socket` is a new file descriptor for this specific client.
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1) {
            perror("accept failed");
            // If accept fails critically, we might want to exit or handle it gracefully.
            // For now, continue to next iteration after logging.
            continue;
        }

        printf("Main thread: Connection accepted from %s:%d. Creating new thread...\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Allocate memory for the new_socket descriptor to pass it to the thread.
        // This is crucial because `new_socket` itself is a local variable
        // and its value will change in the next iteration of the loop.
        int *new_sock_ptr = malloc(sizeof(int));
        if (new_sock_ptr == NULL) {
            perror("malloc failed for new_sock_ptr");
            close(new_socket); // Close the accepted socket if memory allocation fails
            continue;
        }
        *new_sock_ptr = new_socket; // Store the socket descriptor in the allocated memory

        // Create a new thread to handle the client connection
        // pthread_create(thread_id_ptr, attributes, start_routine, arg)
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock_ptr) < 0) {
            perror("could not create thread");
            close(new_socket); // Close the accepted socket if thread creation fails
            free(new_sock_ptr); // Free allocated memory
            continue;
        }

        // Detach the thread. This means the thread's resources will be
        // automatically reclaimed by the system when it exits, without
        // the main thread needing to call pthread_join().
        // This is suitable for server models where the main thread doesn't
        // need to wait for individual client handlers to complete.
        pthread_detach(thread_id);
    }

    // Close the server socket (unreachable in this infinite loop, but good practice)
    close(server_fd);
    return 0;
}

