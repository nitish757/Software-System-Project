#include "common.h"
#include "auth_server.h"
#include "customer_s.h"
#include "employee_s.h"
#include "manager_s.h"
#include "admin_s.h"


#define PORT 8080
#define BUFFER_SIZE 1024

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);  // Reap child processes
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int module_choice;
    struct Customer customer;
    struct Employee employee;
    struct Manager manager;


    while(1){

    read(client_socket, &module_choice, sizeof(module_choice));

    switch (module_choice) {
        case 1:
            printf("Customer Module Selected\n");
            customer_module(client_socket, &customer);
            break;
        case 2:
            printf("Employee Module Selected\n");
            employee_module(client_socket, &customer);
            break;
        case 3:
            printf("Manager Module Selected\n");
            manager_module(client_socket, &manager);
            break;
        case 4:
            printf("Admin module Selected\n");
            admin_module(client_socket, &manager);
            break;
        case 5:
            printf("Client chose to exit.\n");
            close(client_socket);
            exit(0);  
        default:
            printf("Invalid choice: %d\n", module_choice);
            break;
    }
    }

    close(client_socket);  // Close the client socket
    exit(0);  // Exit child after handling client
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pid_t child_pid;

    // Set up signal handler to avoid zombie processes
    signal(SIGCHLD, sigchld_handler);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to an address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;  // Handle next client
        }

        printf("Connected to a client.\n");

        // Fork a child process to handle the client
        if ((child_pid = fork()) == 0) {
            // In child process
            close(server_socket);  // Close unused server socket
            handle_client(client_socket);  // Handle client interaction
        }

        // In parent process
        close(client_socket);  // Close client socket in parent
    }

    close(server_socket);  // Close server socket (this point is never reached)
    return 0;
}
