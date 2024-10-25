#ifndef ADMIN_C_H
#define ADMIN_C_H
#include "common.h"
#include "employee_s.h"

#define BUFFER_SIZE 1024

void add_employee(int client_socket)
{
    char username[50], password[50], line[BUFFER_SIZE];
    int salary, found = 0;

    // Read username, password, and salary from the client
    read(client_socket, username, sizeof(username));
    read(client_socket, password, sizeof(password));
    read(client_socket, &salary, sizeof(salary));

    // Open the employee.txt file for reading
    int file = open("employee.txt", O_RDONLY);
    if (file < 0)
    {
        perror("Error opening file for reading");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    // Check if the username already exists
    while (read(file, line, sizeof(line) - 1) > 0)
    {
        line[sizeof(line) - 1] = '\0'; // Null-terminate the string

        // Parse the line to get the stored username
        char stored_username[50], stored_password[50];
        int stored_salary;
        int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_salary);

        // Check if the stored username matches the new username
        if (items == 3 && strcmp(stored_username, username) == 0)
        {
            found = 1; // Username found
            break;
        }
    }

    close(file); // Close the file after reading

    // If username exists, send error message to client
    if (found)
    {
        send(client_socket, "Username already exists.\n", strlen("Username already exists.\n"), 0);
        return;
    }

    // If username does not exist, open the employee file for appending
    file = open("employee.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file < 0)
    {
        perror("Error opening file for writing");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    // Create the new employee record as a string
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s %s %d\n", username, password, salary);

    // Write the employee data to the file
    if (write(file, buffer, strlen(buffer)) < 0)
    {
        perror("Error writing to file");
        close(file);
        return;
    }

    printf("Employee added successfully.\n");

    // Send success message back to the client
    send(client_socket, "Employee added successfully.\n", strlen("Employee added successfully.\n"), 0);

    // Close the file descriptor
    close(file);
}

void modify_employee_and_customer(int client_socket)
{
    char username[50], new_password[50];
    int new_salary, choice;

    read(client_socket, &choice, sizeof(choice));

    read(client_socket, username, sizeof(username));
    read(client_socket, new_password, sizeof(new_password));
    read(client_socket, &new_salary, sizeof(new_salary));

    if (choice)
        modify_cust_or_emp(client_socket, "employee.txt", username, new_password, new_salary);
    else
        modify_cust_or_emp(client_socket, "customer.txt", username, new_password, new_salary);
}

void change_admin_password(int client_socket, struct Manager *admin) {
    char line[BUFFER_SIZE];
    char new_password[50];
    int found = 0;

    read(client_socket, new_password, sizeof(new_password));

    int file = open("admin.txt", O_RDWR);

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

            if (items == 2 && strcmp(stored_username, admin->username) == 0) {
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1) {
                    perror("Failed to lock record");
                    close(file);
                    return;
                }

                strcpy(admin->password, new_password);

                lseek(file, line_start, SEEK_SET);
                dprintf(file, "%s %s\n", stored_username, admin->password);

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


void admin_module(int client_socket, struct Manager *admin)
{

    if (!authenticate_employee(client_socket, admin, "admin.txt"))
    {
        printf("Authentication failed.\n");
        // close(client_socket);
        // exit(1);
    }
    else
    {
        printf("Authentication Success.\n");
    }

    while (1)
    {
        int op_choice;
        read(client_socket, &op_choice, sizeof(op_choice));

        switch (op_choice)
        {
        case 1:
            add_employee(client_socket);
            break;
        case 2:
            modify_employee_and_customer(client_socket);
            break;
        case 4:
            change_admin_password(client_socket, admin);
            break;
        case 5:
            return;
        case 6:
            return;
            // default:
            //     send(client_socket, "Invalid Choice\n", 15, 0);
        }
    }
}

#endif