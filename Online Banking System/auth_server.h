#ifndef AUTH_SERVER_H
#define AUTH_SERVER_H
#include "common.h"

#define BUFFER_SIZE 1024

int authenticate(int client_socket, struct Customer *customer, char *filename) {
    char buffer[BUFFER_SIZE];
    struct Customer new_cust = {0}; 
    int flag=1;
    // Read username and password from the client
    read(client_socket, new_cust.username, sizeof(new_cust.username));
    read(client_socket, new_cust.password, sizeof(new_cust.password));

    int file = open(filename, O_RDONLY);
    if (file < 0) {
        perror("Failed to open user credentials file");
        return 0; // Authentication failed
    }

    // Read the file line by line
    while (read(file, buffer, sizeof(buffer) - 1) > 0) {
        buffer[BUFFER_SIZE - 1] = '\0'; // Null-terminate the string

        // Parse each line
        char stored_username[50] = {0}; // Initialize to zero
        char stored_password[50] = {0}; // Initialize to zero
        int stored_balance = 0;

        // Use strtok to handle line-by-line parsing
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            // Parse the line into username, password, and balance
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the line was parsed successfully
            if (items == 3) {
                // Check if credentials match
                if (strcmp(stored_username, new_cust.username) == 0 && 
                    strcmp(stored_password, new_cust.password) == 0) {
                    new_cust.balance = stored_balance;
                    *customer = new_cust;
                    close(file); 
                    send(client_socket, &flag, sizeof(flag), 0);
                    return 1; // Authentication successful
                }
            }

            line = strtok(NULL, "\n"); // Get the next line
        }
    }

    close(file); 
    flag=0;
    send(client_socket, &flag, sizeof(flag), 0);
    return 0; // Authentication failed
}

int authenticate_employee(int client_socket, struct Manager *employee, char *filename) {
    char buffer[BUFFER_SIZE];
    int flag = 1;

    // Read username and password from the client
    read(client_socket, employee->username, sizeof(employee->username));
    read(client_socket, employee->password, sizeof(employee->password));

    int file = open(filename, O_RDONLY);
    if (file < 0) {
        perror("Failed to open user credentials file");
        return 0; // Authentication failed
    }

    // Read the file line by line
    while (read(file, buffer, sizeof(buffer) - 1) > 0) {
        buffer[BUFFER_SIZE - 1] = '\0'; // Null-terminate the string

        // Use strtok to handle line-by-line parsing
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            char stored_username[50] = {0}; // Initialize to zero
            char stored_password[50] = {0}; // Initialize to zero

            // Parse the line into username and password
            int items = sscanf(line, "%49s %49s", stored_username, stored_password);

            // Check if the line was parsed successfully
            if (items == 2) {
                // Check if credentials match
                if (strcmp(stored_username, employee->username) == 0 && 
                    strcmp(stored_password, employee->password) == 0) {
                    close(file); 
                    send(client_socket, &flag, sizeof(flag), 0);
                    return 1; // Authentication successful
                }
            }

            line = strtok(NULL, "\n"); // Get the next line
        }
    }

    close(file); 
    flag = 0;
    send(client_socket, &flag, sizeof(flag), 0);
    return 0; // Authentication failed
}

#endif
