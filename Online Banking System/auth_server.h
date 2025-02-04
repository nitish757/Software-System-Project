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
    return 0;
}

// int manage_user_login_state(int client_socket, int login_flag) {
//     char buffer[BUFFER_SIZE];
//     char temp_buffer[BUFFER_SIZE];
//     char *login_status_data = NULL; // To hold the entire file content
//     size_t login_status_size = 0;
//     int user_found = 0;
//     int logged_in = 0;
//     int flag = 1;
//     char username[50];

//     // Read username from the client
//     read(client_socket, username, sizeof(username));

//     // Open the login_status.txt file
//     int file = open("login_status.txt", O_RDWR | O_CREAT, 0644);
//     if (file < 0) {
//         perror("Error opening login status file");
//         return -1; // Error
//     }

//     // Read the entire file content into memory
//     ssize_t bytes_read;
//     while ((bytes_read = read(file, buffer, sizeof(buffer) - 1)) > 0) {
//         buffer[bytes_read] = '\0'; // Null-terminate the string
//         login_status_size += bytes_read;

//         // Reallocate memory to hold the entire content
//         login_status_data = realloc(login_status_data, login_status_size + 1);
//         if (login_status_data == NULL) {
//             perror("Error reallocating memory");
//             close(file);
//             return -1; // Error
//         }

//         strcat(login_status_data, buffer); // Append the new data
//     }

//     // Close the file for reading
//     close(file);

//     // Open the file again for writing
//     file = open("login_status.txt", O_WRONLY | O_TRUNC | O_CREAT, 0644);
//     if (file < 0) {
//         perror("Error opening login status file for writing");
//         free(login_status_data);
//         return -1; // Error
//     }

//     // Parse the file content from memory
//     char *line = strtok(login_status_data, "\n");
//     while (line != NULL) {
//         char stored_username[50] = {0};
//         char stored_status[2] = {0};

//         // Parse the line into username and status
//         int items = sscanf(line, "%49s %1s", stored_username, stored_status);
//         if (items == 2) {
//             // Check if the credentials match
//             if (strcmp(stored_username, username) == 0) {
//                 user_found = 1;
//                 logged_in = (stored_status[0] == '1');

//                 if (login_flag == 1 && logged_in) {
//                     printf("User '%s' is already logged in.\n", username);
//                     flag = 0; // Already logged in
//                     send(client_socket, &flag, sizeof(flag), 0);
//                     free(login_status_data);
//                     close(file);
//                     return 1; // Login successful
//                 } else if (login_flag == 1 && !logged_in) {
//                     // Update the user status to logged in
//                     dprintf(file, "%s 1\n", username); // Write new status
//                     printf("User '%s' logged in successfully.\n", username);
//                 } else if (login_flag == 0) {
//                     // Update the user status to logged out
//                     dprintf(file, "%s 0\n", username); // Write new status
//                     printf("User '%s' logged out successfully.\n", username);
//                 }
//             } else {
//                 // Write the original line back to the file
//                 dprintf(file, "%s\n", line);
//             }
//         }
//         line = strtok(NULL, "\n"); // Get the next line
//     }

//     // If the user was not found and trying to log in
//     if (!user_found && login_flag == 1) {
//         dprintf(file, "%s 1\n", username);
//         printf("User '%s' logged in successfully.\n", username);
//     }

//     // Free the allocated memory and close the file
//     free(login_status_data);
//     close(file);
//     send(client_socket, &flag, sizeof(flag), 0);
//     return (login_flag == 1) ? 1 : 0; 
// }


#endif
