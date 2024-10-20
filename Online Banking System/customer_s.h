#ifndef CUSTOMER_S_H
#define CUSTOMER_S_H
#include "common.h"

#define BUFFER_SIZE 1024

void view_balance(int socket, struct Customer *customer) {
    // Send the balance to the client
    if (send(socket, &customer->balance, sizeof(customer->balance), 0) < 0) {
        perror("Failed to send balance");
    } else {
        printf("Sent balance of %d to user %s\n", customer->balance, customer->username);
    }
}

void deposit_money(int client_socket, struct Customer *customer) {
    char line[BUFFER_SIZE];
    int deposit, found = 0;

    // Read the deposit amount from the client
    read(client_socket, &deposit, sizeof(deposit));

    // Open the file for reading and writing
    int file = open("customer.txt", O_RDWR);
    if (file < 0) {
        perror("Failed to open customer file");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;  // Write lock
    lock.l_whence = SEEK_SET;

    off_t line_start = 0;
    ssize_t bytes_read;
    // Process the file line by line
    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        // Null-terminate the string
        line[bytes_read] = '\0'; 

        // Find the position of the newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            // Calculate the actual length of the line
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR);

            // Variables to hold parsed data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches
            if (items == 3 && strcmp(stored_username, customer->username) == 0) {
                // Set the lock to cover this line
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1) {
                    perror("Failed to lock record");
                    close(file);
                    return;
                }

                // Update the balance
                stored_balance += deposit;

                // Move back to the start of the line and overwrite it
                lseek(file, line_start, SEEK_SET);
                dprintf(file, "%s %s %d\n", stored_username, stored_password, stored_balance);

                // Unlock the record
                lock.l_type = F_UNLCK;
                fcntl(file, F_SETLK, &lock);

                // Update the balance in memory
                customer->balance = stored_balance;
                send(client_socket, "Deposit successful\n", 20, 0);
                found = 1;
                break;
            }

            // Move to the next line after the newline character
            lseek(file, line_start + line_length, SEEK_SET);
        } else {
            // Move file pointer forward if no newline character is found
            lseek(file, line_start + bytes_read, SEEK_SET);
        }
    }

    if (!found) {
        send(client_socket, "User not found\n", 16, 0);
    }

    close(file);
}

void withdraw_money(int client_socket, struct Customer *customer) {
    char line[BUFFER_SIZE];
    int withdraw, found = 0;

    // Read the withdrawal amount from the client
    read(client_socket, &withdraw, sizeof(withdraw));

    // Open the file for reading and writing
    int file = open("customer.txt", O_RDWR);
    if (file < 0) {
        perror("Failed to open customer file");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;  // Write lock
    lock.l_whence = SEEK_SET;

    off_t line_start = 0;
    ssize_t bytes_read;

    // Process the file line by line
    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        // Null-terminate the string
        line[bytes_read] = '\0'; 

        // Find the position of the newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            // Calculate the actual length of the line
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR);

            // Variables to hold parsed data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches
            if (items == 3 && strcmp(stored_username, customer->username) == 0) {
                // Check if sufficient balance is available
                if (stored_balance >= withdraw) {
                    // Set the lock to cover this line
                    lock.l_start = line_start;
                    lock.l_len = line_length;

                    if (fcntl(file, F_SETLKW, &lock) == -1) {
                        perror("Failed to lock record");
                        close(file);
                        return;
                    }

                    // Update the balance
                    stored_balance -= withdraw;

                    // Move back to the start of the line and overwrite it
                    lseek(file, line_start, SEEK_SET);
                    dprintf(file, "%s %s %d\n", stored_username, stored_password, stored_balance);

                    // Unlock the record
                    lock.l_type = F_UNLCK;
                    fcntl(file, F_SETLK, &lock);

                    // Update the customer's balance in memory
                    customer->balance = stored_balance;
                    send(client_socket, "Withdrawal successful\n", 22, 0);
                } else {
                    send(client_socket, "Insufficient funds\n", 20, 0);
                }
                found = 1;
                break;
            }

            // Move to the next line after the newline character
            lseek(file, line_start + line_length, SEEK_SET);
        } else {
            // Move file pointer forward if no newline character is found
            lseek(file, line_start + bytes_read, SEEK_SET);
        }
    }

    if (!found) {
        send(client_socket, "User not found\n", 16, 0);
    }

    close(file);
}

void transfer_funds(int client_socket, struct Customer *sender) {
    char receiver_username[50];
    int transfer_amount;

    // Read the receiver's username and transfer amount from the client
    read(client_socket, receiver_username, sizeof(receiver_username));
    read(client_socket, &transfer_amount, sizeof(transfer_amount));

    char line[BUFFER_SIZE];
    int found_sender = 0, found_receiver = 0;
    int sender_balance = 0, receiver_balance = 0;

    // Open the file for reading and writing
    int file = open("customer.txt", O_RDWR);
    if (file < 0) {
        perror("Failed to open customer file");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;  // Write lock

    // First pass: Find the sender and update their balance
    off_t line_start = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the string

        // Find the position of the newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            // Calculate the actual length of the line
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR); // Save current position

            // Variables to hold parsed data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches the sender
            if (items == 3 && strcmp(stored_username, sender->username) == 0) {
                found_sender = 1; // Mark sender as found
                // Check if sufficient balance is available
                if (stored_balance >= transfer_amount) {
                    // Set the lock to cover this line
                    lock.l_start = line_start;
                    lock.l_len = line_length;

                    if (fcntl(file, F_SETLKW, &lock) == -1) {
                        perror("Failed to lock sender record");
                        close(file);
                        return;
                    }

                    // Update the sender's balance
                    stored_balance -= transfer_amount;

                    // Move back to the start of the line and overwrite it
                    lseek(file, line_start, SEEK_SET);
                    dprintf(file, "%s %s %d\n", stored_username, stored_password, stored_balance);

                    // Unlock the sender record
                    lock.l_type = F_UNLCK;
                    fcntl(file, F_SETLK, &lock);

                    // Update sender's balance in memory
                    sender->balance = stored_balance;

                } else {
                    send(client_socket, "Insufficient funds\n", 20, 0);
                    close(file);
                    return;
                }
            }
            // Move to the next line after the newline character
            lseek(file, line_start + line_length, SEEK_SET);
        } else {
            lseek(file, line_start + bytes_read, SEEK_SET);
        }
    }

    // If sender not found, return
    if (!found_sender) {
        send(client_socket, "Sender not found\n", 17, 0);
        close(file);
        return;
    }

    // Reset file pointer for second pass: Find the receiver and update their balance
    lseek(file, 0, SEEK_SET);

    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the string

        // Find the position of the newline character
        char *newline_pos = strchr(line, '\n');
        if (newline_pos) {
            // Calculate the actual length of the line
            ssize_t line_length = newline_pos - line + 1;
            line_start = lseek(file, -bytes_read, SEEK_CUR); // Save current position

            // Variables to hold parsed data
            char stored_username[50], stored_password[50];
            int stored_balance;

            // Parse the line
            int items = sscanf(line, "%49s %49s %d", stored_username, stored_password, &stored_balance);

            // Check if the username matches the receiver
            if (items == 3 && strcmp(stored_username, receiver_username) == 0) {
                found_receiver = 1; // Mark receiver as found
                
                // Set the lock to cover this line
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1) {
                    perror("Failed to lock receiver record");
                    close(file);
                    return;
                }

                // Update the receiver's balance
                stored_balance += transfer_amount;

                // Move back to the start of the line and overwrite it
                lseek(file, line_start, SEEK_SET);
                dprintf(file, "%s %s %d\n", stored_username, stored_password, stored_balance);

                // Unlock the receiver record
                lock.l_type = F_UNLCK;
                fcntl(file, F_SETLK, &lock);
            }
            // Move to the next line after the newline character
            lseek(file, line_start + line_length, SEEK_SET);
        } else {
            lseek(file, line_start + bytes_read, SEEK_SET);
        }
    }

    // Close the file
    close(file);

    // Notify the client of success or failure
    if (found_receiver) {
        send(client_socket, "Transfer successful\n", 20, 0);
    } else {
        send(client_socket, "Receiver not found\n", 20, 0);
    }
}

void change_password(int client_socket, struct Customer *customer) {
    char line[BUFFER_SIZE];
    char new_password[50];
    int found = 0;

    // Read the new password from the client
    read(client_socket, new_password, sizeof(new_password));

    // Open the customer.txt file for reading and writing
    int file = open("customer.txt", O_RDWR);
    if (file < 0) {
        perror("Failed to open customer file");
        send(client_socket, "Error opening file\n", 20, 0);
        return;
    }

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;  // Write lock
    lock.l_whence = SEEK_SET;

    off_t line_start = 0;
    ssize_t bytes_read;

    // Process the file line by line
    while ((bytes_read = read(file, line, sizeof(line) - 1)) > 0) {
        // Null-terminate the string
        line[bytes_read] = '\0';

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
            if (items == 3 && strcmp(stored_username, customer->username) == 0) {
                // Set a write lock on the matched line
                lock.l_start = line_start;
                lock.l_len = line_length;

                if (fcntl(file, F_SETLKW, &lock) == -1) {
                    perror("Failed to lock record");
                    close(file);
                    return;
                }

                // Update the password
                strcpy(customer->password, new_password);

                // Move to the start of the line and overwrite it
                lseek(file, line_start, SEEK_SET);
                dprintf(file, "%s %s %d\n", stored_username, customer->password, stored_balance);

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

    if (!found) {
        send(client_socket, "User not found\n", 16, 0);
    }

    close(file);
}

void customer_module(int client_socket, struct Customer *customer) {
    int op_choice;

    if (!authenticate(client_socket, customer, "customer.txt")) {
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
            view_balance(client_socket, customer);
            break;
        case 2:
            deposit_money(client_socket, customer);
            break;
        case 3:
            withdraw_money(client_socket, customer);
            break;
        case 4:
            transfer_funds(client_socket, customer);
            break;
        case 6:
            change_password(client_socket, customer);
            break;
        default:
            send(client_socket, "Invalid Choice\n", 15, 0);
    }
}
#endif