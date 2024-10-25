#ifndef EMPLOYEE_S_H
#define EMPLOYEE_S_H
#include "common.h"

#define BUFFER_SIZE 1024

void add_customer(int client_socket)
{
    char username[50], password[50];
    char line[BUFFER_SIZE];
    int balance;
    int found = 0;

    // Read username, password, and balance from the client
    read(client_socket, username, sizeof(username));
    read(client_socket, password, sizeof(password));
    read(client_socket, &balance, sizeof(balance));

    // Open the customer.txt file for reading
    int file = open("customer.txt", O_RDONLY);
    if (file < 0)
    {
        perror("Error opening file for reading");
        return;
    }

    // Read the file line by line to check if the username already exists
    ssize_t bytes_read;
    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0)
    {
        line[bytes_read] = '\0'; // Null-terminate the string

        // Process each line individually
        char *line_ptr = line;
        while (line_ptr)
        {
            char *newline_pos = strchr(line_ptr, '\n');
            if (newline_pos)
                *newline_pos = '\0'; // Replace newline with null terminator

            // Extract username from the current line
            char stored_username[50], stored_password[50];
            int stored_balance;
            int items = sscanf(line_ptr, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches
            if (items == 3 && strcmp(stored_username, username) == 0)
            {
                found = 1;
                break;
            }

            // Move to the next line (if any)
            line_ptr = newline_pos ? newline_pos + 1 : NULL;
        }

        if (found)
            break; // Stop reading if username is found
    }
    close(file); // Close the file after reading

    // If username exists, send error message to the client
    if (found)
    {
        send(client_socket, "Username already exists.\n", strlen("Username already exists.\n"), 0);
        return;
    }

    // Open the customer.txt file for appending the new customer record
    file = open("customer.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file < 0)
    {
        perror("Error opening file for writing");
        return;
    }

    // Create the new customer record as a string
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %s %d\n", username, password, balance);

    // Write the new record to the file
    if (write(file, buffer, strlen(buffer)) < 0)
    {
        perror("Error writing to file");
        close(file);
        return;
    }

    printf("Customer added successfully.\n");

    // Send success message back to the client
    send(client_socket, "Customer added successfully.\n", strlen("Customer added successfully.\n"), 0);

    // Close the file descriptor
    close(file);
}

void modify_cust_or_emp(int client_socket, const char *filename, char *username, char *new_password, int new_balance)
{

    // Read username, new password, and new balance from the client
    // read(client_socket, username, sizeof(username));
    // read(client_socket, new_password, sizeof(new_password));
    // read(client_socket, &new_balance, sizeof(new_balance));

    int file = open(filename, O_RDWR);
    if (file < 0)
    {
        perror("Failed to open customer file");
        return;
    }

    char line[BUFFER_SIZE];
    off_t line_start = 0;
    ssize_t bytes_read;
    int found = 0;
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;    // Write lock
    lock.l_whence = SEEK_SET; // Lock relative to the start of the file

    // Read the file line by line
    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0)
    {
        line[bytes_read] = '\0'; // Null-terminate the line

        // Find the newline character to determine the line length
        char *newline_pos = strchr(line, '\n');
        if (newline_pos)
        {
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR); // Move to the start of this line

            // Variables to hold parsed customer data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line into username, password, and balance
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // If the username matches, modify the record
            if (items >= 2 && strcmp(stored_username, username) == 0)
            {
                // Set the lock for this record
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1)
                {
                    perror("Failed to lock the record");
                    close(file);
                    return;
                }

                // Move back to the start of the line
                lseek(file, line_start, SEEK_SET);

                // Write the modified customer details
                dprintf(file, "%s %s %d\n", stored_username, new_password, new_balance);

                // Unlock the record
                lock.l_type = F_UNLCK;
                fcntl(file, F_SETLK, &lock);

                found = 1;
                send(client_socket, "Details updated successfully.\n", sizeof("Details updated successfully.\n"), 0);
                break;
            }

            // Move to the next line
            lseek(file, line_start + line_length, SEEK_SET);
        }
        else
        {
            // If no newline is found, move to the end of the buffer
            lseek(file, line_start + bytes_read, SEEK_SET);
        }
    }

    if (!found)
    {
        send(client_socket, "Customer not found.\n", sizeof("Customer not found.\n"), 0);
    }

    close(file); // Close the file
}

void modify_customer(int client_socket)
{
    char username[50], new_password[50];
    int new_balance;

    read(client_socket, username, sizeof(username));
    read(client_socket, new_password, sizeof(new_password));
    read(client_socket, &new_balance, sizeof(new_balance));

    modify_cust_or_emp(client_socket, "customer.txt", username, new_password, new_balance);
}

void change_emp_password(int client_socket, struct Customer *employee)
{
    char line[BUFFER_SIZE];
    char new_password[50];
    int found = 0;

    // Read the new password from the client
    read(client_socket, new_password, sizeof(new_password));

    // Open the employee.txt file for reading and writing
    int file = open("employee.txt", O_RDWR);
    if (file < 0)
    {
        perror("Failed to open employee file");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET;

    off_t line_start = 0;
    ssize_t bytes_read;

    // Process the file line by line
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
            int stored_salary;

            // Parse the line (now includes salary)
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_salary);

            // Check if the username matches
            if (items == 3 && strcmp(stored_username, employee->username) == 0)
            {
                // Set a write lock on the matched line
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1)
                {
                    perror("Failed to lock record");
                    close(file);
                    return;
                }

                // Move to the start of the line and overwrite it with the new password
                lseek(file, line_start, SEEK_SET);
                dprintf(file, "%s %s %d\n", stored_username, new_password, stored_salary);

                // Unlock the record
                lock.l_type = F_UNLCK;
                fcntl(file, F_SETLK, &lock);

                send(client_socket, "Password changed successfully\n", 30, 0);
                found = 1;
                break;
            }

            // Move to the next line after the newline character
            lseek(file, line_start + line_length, SEEK_SET);
        }
    }

    if (!found)
    {
        send(client_socket, "User not found\n", 16, 0);
    }

    close(file);
}

void process_loan(int client_socket, struct Customer *employee) {
    int loan_id;
    char decision[10];
    // char username[50];
    char buffer[BUFFER_SIZE];
    int is_assigned = 0; // Flag to check if loan is assigned
    ssize_t bytes_read;
    off_t pos;

    // Receive loan ID, employee username, and decision (Approved/Rejected) from the employee
    recv(client_socket, &loan_id, sizeof(loan_id), 0);
    printf("recvd id n decision");
    // recv(client_socket, username, sizeof(username), 0);
    recv(client_socket, decision, sizeof(decision), 0);
    printf("recvd id n decision");
    // Step 1: Verify if the loan is assigned to the correct employee
    int assignment_file = open("loan_assignments.txt", O_RDONLY);
    if (assignment_file < 0) {
        perror("Failed to open loan assignment file");
        send(client_socket, "Error opening assignment file\n", 30, 0);
        return;
    }

    // Read through the loan_assignments.txt using system calls
    while ((bytes_read = read(assignment_file, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';

        // Process each line in the buffer
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            int stored_loan_id;
            char stored_employee[50];

            // Read loan_id and assigned employee's username from the assignment line
            if (sscanf(line, "%d %49s", &stored_loan_id, stored_employee) == 2) {
                // Check if the loan is assigned to this employee based on LoanID and username
                if (stored_loan_id == loan_id && strcmp(stored_employee, employee->username) == 0) {
                    is_assigned = 1;  // Loan is assigned to the employee
                    break;            // Exit the loop
                }
            }
            line = strtok(NULL, "\n");
        }
        if (is_assigned) break;  // Exit if found
    }

    close(assignment_file);  // Close assignment file

    if (!is_assigned) {
        // If the loan is not assigned to this employee, reject the operation
        send(client_socket, "Loan not assigned to you.\n", 27, 0);
        return;
    }

    // Step 2: If the loan is assigned, proceed to update the loan status in loans.txt
    int loan_file = open("loans.txt", O_RDWR);
    if (loan_file < 0) {
        perror("Failed to open loan database");
        send(client_socket, "Error opening loan file\n", 25, 0);
        return;
    }

    // Read through the loans.txt to find and update the loan status
    while ((bytes_read = read(loan_file, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("inside update loan");
        // Process each line individually
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            int stored_loan_id;
            char customer_name[50], loan_status[20];
            int loan_amount;

            // Extract loan details from the line (loanID customerName loanAmount loanStatus)
            if (sscanf(line, "%d %49s %d %19s", &stored_loan_id, customer_name, &loan_amount, loan_status) == 4) {
                if (stored_loan_id == loan_id) {
                    // Update the line with the loan decision (Approved/Rejected)
                    pos = lseek(loan_file, -strlen(line), SEEK_CUR); // Get current position and move back
                    dprintf(loan_file, "%d %s %d %s\n", stored_loan_id, customer_name, loan_amount, decision);
                    printf("Loan %d %s\n", stored_loan_id, decision);
                    break; // Exit after updating the loan
                }
            }
            line = strtok(NULL, "\n");
        }
    }

    close(loan_file);  // Close loan file

    // Send confirmation to the employee
    send(client_socket, "Loan processed successfully.\n", 30, 0);
}

void view_assigned_loans(int client_socket, struct Customer *employee)
{
    char buffer[BUFFER_SIZE];
    char loan_buffer[BUFFER_SIZE];
    int loan_file;
    ssize_t bytes_read;

    // Open the loan assignment file to get the loans assigned to the employee
    int assignment_file = open("loan_assignments.txt", O_RDONLY);
    if (assignment_file < 0)
    {
        perror("Failed to open loan assignment file");
        return;
    }

    // Buffer to hold assigned loans
    char assigned_loans[BUFFER_SIZE] = "";

    // Read through the assignments
    while ((bytes_read = read(assignment_file, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0'; // Null-terminate the buffer

        // Process each line in loan_assignments.txt
        char *line = strtok(buffer, "\n");
        while (line != NULL)
        {
            int stored_loan_id;
            char stored_employee[50];

            // Read loan_id and employee_username from the assignment line
            if (sscanf(line, "%d %49s", &stored_loan_id, stored_employee) == 2)
            {
                // Check if the assignment matches the employee
                if (strcmp(stored_employee, employee->username) == 0)
                {
                    // If assigned to this employee, store the loan ID
                    char loan_info[20];
                    snprintf(loan_info, sizeof(loan_info), "%d ", stored_loan_id);
                    strncat(assigned_loans, loan_info, sizeof(assigned_loans) - strlen(assigned_loans) - 1);
                }
            }
            line = strtok(NULL, "\n");
        }
    }
    close(assignment_file); // Close assignment file

    // If no loans assigned, inform the employee
    if (strlen(assigned_loans) == 0)
    {
        send(client_socket, "No loans assigned to you.\n", 25, 0);
        return;
    }

    // Open the loans database to fetch loan details
    loan_file = open("loans.txt", O_RDONLY);
    if (loan_file < 0)
    {
        perror("Failed to open loan database");
        return;
    }

    // Read the loan details
    char *token = strtok(assigned_loans, " ");
    char response[BUFFER_SIZE] = "Assigned Loans:\n";

    while (token != NULL)
    {
        // For each loan ID, fetch the details from loans.txt
        while ((bytes_read = read(loan_file, loan_buffer, sizeof(loan_buffer) - 1)) > 0)
        {
            loan_buffer[bytes_read] = '\0'; // Null-terminate the buffer

            // Process each line in loans.txt
            char *loan_line = strtok(loan_buffer, "\n");
            while (loan_line != NULL)
            {
                int stored_loan_id;
                char customer_name[50];
                int loan_amount;
                char loan_status[20];

                // Read loan_id, customer_name, loan_amount, and status from the loan line
                if (sscanf(loan_line, "%d %49s %d %19s", &stored_loan_id, customer_name, &loan_amount, loan_status) == 4)
                {
                    if (stored_loan_id == atoi(token))
                    {
                        // Append loan details to the response
                        char loan_detail[200];
                        snprintf(loan_detail, sizeof(loan_detail), "Loan ID: %d, Customer: %s, Amount: %d, Status: %s\n",
                                 stored_loan_id, customer_name, loan_amount, loan_status);
                        strncat(response, loan_detail, sizeof(response) - strlen(response) - 1);
                        break; // Exit after finding the loan details
                    }
                }
                loan_line = strtok(NULL, "\n");
            }
        }
        token = strtok(NULL, " ");
    }
    close(loan_file); // Close the loan database file

    // Send the loan details back to the employee
    send(client_socket, response, strlen(response), 0);
}

void employee_module(int client_socket, struct Customer *employee)
{

    if (!authenticate(client_socket, employee, "employee.txt")){
        printf("Authentication failed.\n");
    }
    else{
        printf("Authentication Success.\n");
    }

    while (1){
        int op_choice;
        read(client_socket, &op_choice, sizeof(op_choice));

        switch (op_choice){
        case 1:
            add_customer(client_socket);
            break;
        case 2:
            modify_customer(client_socket);
            break;
        case 3:
            process_loan(client_socket, employee);
            break;
        case 4:
            view_assigned_loans(client_socket, employee);
            break;
        case 5:
            change_emp_password(client_socket, employee);
            break;
        case 6:
            return;
        case 7:
            // close(client_socket);
            break;
        // case 3:
        //     withdraw_money(client_socket, customer);
        //     break;
        // case 4:
        //     transfer_funds(client_socket, customer);
        //     break;
        // case 6:
        //     change_password(client_socket, customer);
        //     break;
        default:
            send(client_socket, "Invalid Choice\n", 15, 0);
        }
    }
}

#endif