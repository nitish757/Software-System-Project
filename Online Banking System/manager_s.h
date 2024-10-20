#ifndef MANAGER_S_H
#define MANAGER_S_H
#include "common.h"

#define BUFFER_SIZE 1024

void deactivate_func(int client_socket, const char *filename, const char *deact_filename) {
    char line[BUFFER_SIZE], username[50];
    int found = 0;

    read(client_socket, username, sizeof(username));

    int file = open(filename, O_RDWR);
    if (file < 0) {
        perror("Failed to open customer file");
        return;
    }

    // Open the deactivated_customers.txt file for appending
    int deactivated_file = open(deact_filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (deactivated_file < 0) {
        perror("Failed to open deactivated customers file");
        close(file);
        return;
    }

    // Process the file line by line
    off_t line_start = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the string

        // Find the position of the newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR);

            // Variables to hold parsed data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches
            if (items == 3 && strcmp(stored_username, username) == 0) {
                // Write to the deactivated customers file
                write(deactivated_file, line, line_length);

                // Overwrite the customer's line in the original file with spaces
                lseek(file, line_start, SEEK_SET);
                char empty_line[BUFFER_SIZE];
                memset(empty_line, ' ', line_length); // Clear the line
                empty_line[line_length - 1] = '\n'; // Ensure it ends with a newline
                write(file, empty_line, line_length); // Remove line

                found = 1;
                break; // Exit the loop as the customer is deactivated
            }

            // Move to the next line after the newline character
            lseek(file, line_start + line_length, SEEK_SET);
        }
    }

    // Close files
    close(file);
    close(deactivated_file);

    if (!found) {
        send(client_socket, "User not found\n", 16, 0);
    } else {
        send(client_socket, "Successfull\n", 34, 0);
    }
}

void activate_deactivate_customer(int client_socket){
    int active;

    read(client_socket, &active, sizeof(active));

    if(active)
        deactivate_func(client_socket,"deactivated_customers.txt","customer.txt");
    else
        deactivate_func(client_socket, "customer.txt", "deactivated_customers.txt");
}
void manager_module(int client_socket, struct Manager *manager){
    int op_choice;

    if (!authenticate_employee(client_socket, manager, "manager.txt")) {
        printf("Authentication failed.\n");
        close(client_socket);
        exit(1);
    }
    else{
        printf("Authentication Success.\n");
    }

    read(client_socket, &op_choice, sizeof(op_choice)); 

    switch (op_choice) {
        case 1:
            activate_deactivate_customer(client_socket);
            break;
        // case 2:
        //     modify_customer(client_socket);
        //     break;
        // case 6:
        //     change_emp_password(client_socket, employee);
        //     break;
        // default:
        //     send(client_socket, "Invalid Choice\n", 15, 0);
    }
}

#endif