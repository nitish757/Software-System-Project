#ifndef MANAGER_S_H
#define MANAGER_S_H
#include "common.h"

#define BUFFER_SIZE 1024

void deactivate_func(int client_socket, const char *filename, const char *deact_filename)
{
    char line[BUFFER_SIZE], username[50];
    int found = 0;

    read(client_socket, username, sizeof(username));

    int file = open(filename, O_RDWR);
    if (file < 0)
    {
        perror("Failed to open customer file");
        return;
    }

    // Open the deactivated_customers.txt file for appending
    int deactivated_file = open(deact_filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (deactivated_file < 0)
    {
        perror("Failed to open deactivated customers file");
        close(file);
        return;
    }

    // Process the file line by line
    off_t line_start = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0)
    {
        line[bytes_read] = '\0'; // Null-terminate the string

        // Find the position of the newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos)
        {
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR);

            // Variables to hold parsed data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches
            if (items == 3 && strcmp(stored_username, username) == 0)
            {
                // Write to the deactivated customers file
                write(deactivated_file, line, line_length);

                // Overwrite the customer's line in the original file with spaces
                lseek(file, line_start, SEEK_SET);
                char empty_line[BUFFER_SIZE];
                memset(empty_line, ' ', line_length); // Clear the line
                empty_line[line_length - 1] = '\n';   // Ensure it ends with a newline
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

    if (!found)
    {
        send(client_socket, "User not found\n", 16, 0);
    }
    else
    {
        send(client_socket, "Successfull\n", 34, 0);
    }
}

void activate_deactivate_customer(int client_socket)
{
    int active;

    read(client_socket, &active, sizeof(active));

    if (active)
        deactivate_func(client_socket, "deactivated_customers.txt", "customer.txt");
    else
        deactivate_func(client_socket, "customer.txt", "deactivated_customers.txt");
}

void change_man_password(int client_socket, struct Manager *manager) {
    char line[BUFFER_SIZE];
    char new_password[50];
    int found = 0;

    read(client_socket, new_password, sizeof(new_password));

    int file = open("manager.txt", O_RDWR);

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;  // Write lock
    lock.l_whence = SEEK_SET;

    off_t line_start = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0';

        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR);

            char stored_username[50], stored_password[50];

            int items = sscanf(line, "%49s %49s", stored_username, stored_password);

            if (items == 2 && strcmp(stored_username, manager->username) == 0) {
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1) {
                    perror("Failed to lock record");
                    close(file);
                    return;
                }

                strcpy(manager->password, new_password);

                lseek(file, line_start, SEEK_SET);
                dprintf(file, "%s %s\n", stored_username, manager->password);

                lock.l_type = F_UNLCK;
                fcntl(file, F_SETLK, &lock);

                send(client_socket, "Password changed successfully\n", 30, 0);
                found = 1;
                break;
            }

            lseek(file, line_start + line_length, SEEK_SET);
        }
    }
    if (!found) {
        send(client_socket, "User not found\n", 16, 0);
    }

    close(file);
}


void assign_loan(int client_socket)
{
    int loan_id;
    char username[50];
    char buffer[BUFFER_SIZE];

    // Receive loan ID and employee username from manager
    recv(client_socket, &loan_id, sizeof(loan_id), 0);
    recv(client_socket, username, sizeof(username), 0);

    // Open loan database file for reading (loans.txt)
    int file = open("loans.txt", O_RDONLY);
    if (file < 0)
    {
        perror("Failed to open loan database");
        return;
    }

    ssize_t bytes_read;
    int found = 0; // Flag to check if loan ID is found

    // Read through the loans.txt file to check if the loan ID exists
    while ((bytes_read = read(file, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';

        // Process each line individually
        char *line = strtok(buffer, "\n");
        while (line != NULL)
        {
            int stored_loan_id;
            sscanf(line, "%d", &stored_loan_id);

            // Check if the loan ID matches
            if (stored_loan_id == loan_id)
            {
                found = 1; // Loan ID found
                break;
            }
            line = strtok(NULL, "\n");
        }
    }

    close(file); // Close the loans.txt file after reading

    if (found)
    {
        // Loan ID is found, proceed to store the assignment in loan_assignments.txt
        int assignment_file = open("loan_assignments.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (assignment_file < 0)
        {
            perror("Failed to open loan assignment file");
            return;
        }

        // Append the loan ID and employee username to loan_assignments.txt
        dprintf(assignment_file, "%d %s\n", loan_id, username);
        printf("Loan %d assigned to employee %s.\n", loan_id, username);

        // Close the loan_assignments.txt file
        close(assignment_file);

        // Send confirmation to manager
        send(client_socket, "Loan assigned successfully.\n", 30, 0);
    }
    else
    {
        // If loan ID was not found
        send(client_socket, "Loan ID not found.\n", 19, 0);
    }
}

void manager_module(int client_socket, struct Manager *manager)
{
    // int op_choice;

    if (!authenticate_employee(client_socket, manager, "manager.txt"))
    {
        printf("Authentication failed.\n");
        // close(client_socket);
        // exit(1);
    }
    else
    {
        printf("Authentication Success.\n");
    }

    while (1){
        int op_choice;
        read(client_socket, &op_choice, sizeof(op_choice));
        switch (op_choice){
        case 1:
            activate_deactivate_customer(client_socket);
            break;
        case 2:
            assign_loan(client_socket);
            break;
        case 4:
            change_man_password(client_socket, manager);
            break;
        case 5:
            return;
        case 6:
            return;
            // case 6:
            //     change_emp_password(client_socket, employee);
            //     break;
            // default:
            //     send(client_socket, "Invalid Choice\n", 15, 0);
        }
    }
}

#endif