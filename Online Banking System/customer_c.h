#ifndef CUSTOMER_C_H
#define CUSTOMER_C_H

#define BUFFER_SIZE 1024

void customer_menu()
{
    printf("\n\n------------Customer Module:--------------\n");
    printf("1. View Balance\n");
    printf("2. Deposit Money\n");
    printf("3. Withdraw Money\n");
    printf("4. Transfer Fund\n");
    printf("5. Apply for a Loan\n");
    printf("6. Change Password\n");
    printf("7. Adding Feedback\n");
    printf("8. View Transaction History\n");
    printf("9. Logout\n");
    printf("10. Exit\n\n");
}

void view_balance(int socket)
{
    int balance;
    read(socket, &balance, sizeof(balance));
    printf("\nCurrent Balance: %d\n", balance);
}

void deposit_money(int socket)
{
    int deposit;
    char response[50];

    printf("\nEnter amount to deposit:");
    scanf("%d", &deposit);

    send(socket, &deposit, sizeof(deposit), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void withdraw_money(int socket)
{
    int withdraw;
    char response[50];

    printf("\nEnter amount to withdraw:");
    scanf("%d", &withdraw);

    send(socket, &withdraw, sizeof(withdraw), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void transfer_funds(int socket)
{
    int transfer_amount;
    char recv_username[50];
    char response[50];

    printf("\nEnter reciever username:");
    scanf("%s", recv_username);
    send(socket, recv_username, sizeof(recv_username), 0);

    printf("\nEnter amount to transfer:");
    scanf("%d", &transfer_amount);
    send(socket, &transfer_amount, sizeof(transfer_amount), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void change_password(int socket)
{
    char new_password[50], response[50];

    printf("\nEnter new password:");
    scanf("%s", new_password);

    send(socket, new_password, sizeof(new_password), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void add_feedback(int sock)
{
    char feedback[BUFFER_SIZE];
    char message[BUFFER_SIZE + 50]; // Buffer to hold the final message

    printf("Enter your feedback: ");

    // Read feedback from the user
    fflush(stdout); // Flush output buffer to ensure the prompt is displayed
    if (fgets(feedback, sizeof(feedback), stdin) == NULL)
    {
        perror("Error reading feedback");
        return;
    }

    // Remove newline character at the end, if present
    size_t len = strlen(feedback);
    if (len > 0 && feedback[len - 1] == '\n')
    {
        feedback[len - 1] = '\0';
    }

    // Build the message to send to the server (e.g., just feedback in this case)
    strcpy(message, feedback);

    // Send feedback to the server
    if (send(sock, message, strlen(message), 0) < 0)
    {
        perror("Error sending feedback");
        return;
    }

    // Receive confirmation from the server
    char server_response[BUFFER_SIZE];
    int bytes_received = recv(sock, server_response, sizeof(server_response) - 1, 0);
    if (bytes_received < 0)
    {
        perror("Error receiving response from server");
        return;
    }

    // Null-terminate and display the server response
    server_response[bytes_received] = '\0';
    printf("Server: %s\n", server_response);
}

void apply_loan(int client_socket)
{
    char loan_details[256], response[50];
    int loan_amount;

    // Get loan amount and details from customer
    printf("Enter loan amount: ");
    scanf("%d", &loan_amount);

    // Send loan details to the server
    send(client_socket, &loan_amount, sizeof(loan_amount), 0);

    recv(client_socket, response, sizeof(response), 0);
    printf("%s\n", response);
}

void customer_module(int socket)
{
    // int op_choice;

    // login_status(int sock, char *username)

    while (1){
        int op_choice;
        customer_menu();

        printf("Enter your choice of Operation:");
        scanf("%d", &op_choice);

        send(socket, &op_choice, sizeof(op_choice), 0);

        switch (op_choice){
        case 1:
            view_balance(socket);
            break;
        case 2:
            deposit_money(socket);
            break;
        case 3:
            withdraw_money(socket);
            break;
        case 4:
            transfer_funds(socket);
            break;
        case 5:
            apply_loan(socket);
            break;
        case 6:
            change_password(socket);
            break;
        case 7:
            add_feedback(socket);
            break;
        case 9:
            return;
        case 10:
            // close(socket);
            return;
        }
    }
}
#endif