#ifndef EMPLOYEE_C_H
#define EMPLOYEE_C_H
#include "common.h"

#define BUFFER_SIZE 1024

void employee_menu()
{
    printf("\n\n------------Employee Module:--------------\n");
    printf("1. Add New Custmomer\n");
    printf("2. Modify Customer Details\n");
    printf("3. Process Loan Applications (Approve/Reject Loans)\n");
    printf("4. View Assigned Loan Applications\n");
    printf("5. Change Password\n");
    printf("6. Logout\n");
    printf("7. Exit\n\n");
}

void add_customer(int socket)
{
    char username[50], password[50], response[50];
    int balance;

    printf("\nEnter username of new customer:");
    scanf("%s", username);
    send(socket, username, sizeof(username), 0);

    printf("\nEnter password of new customer:");
    scanf("%s", password);
    send(socket, password, sizeof(password), 0);

    printf("\nEnter balance of new customer:");
    scanf("%d", &balance);
    send(socket, &balance, sizeof(balance), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void modify_customer(int socket)
{
    char username[50], password[50], response[50];
    int balance;

    printf("\nEnter username of customer:");
    scanf("%s", username);
    send(socket, username, sizeof(username), 0);

    printf("\nEnter new password of customer:");
    scanf("%s", password);
    send(socket, password, sizeof(password), 0);

    printf("\nEnter new balance of customer:");
    scanf("%d", &balance);
    send(socket, &balance, sizeof(balance), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void change_emp_password(int socket)
{
    char new_password[50], response[50];

    printf("\nEnter new password:");
    scanf("%s", new_password);

    send(socket, new_password, sizeof(new_password), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void process_loan(int socket)
{
    int loan_id;
    char decision[10], username[50];

    printf("Enter Loan ID to process: ");
    scanf("%d", &loan_id);
    // printf("Enter the Username of employee: ");
    // scanf("%s", decision);
    printf("Approve or Reject: ");
    scanf("%s", decision);

    // Send loan ID and decision to server
    send(socket, &loan_id, sizeof(loan_id), 0);
    //send(socket, username, sizeof(username), 0);
    send(socket, decision, sizeof(decision), 0);

    // Receive confirmation from server
    char response[50];
    recv(socket, response, sizeof(response), 0);
    printf("%s\n", response);
}

void view_assigned_loans(int sock)
{
    // char employee_username[50];
    char response[BUFFER_SIZE];

    // Prompt for employee username
    // printf("Enter your username: ");
    // scanf("%49s", employee_username); // Read username safely

    // // Send username to server
    // send(sock, employee_username, sizeof(employee_username), 0);

    // Receive loan details from the server
    ssize_t bytes_received = recv(sock, response, sizeof(response) - 1, 0);
    if (bytes_received < 0)
    {
        perror("Error receiving loan details");
        return;
    }

    response[bytes_received] = '\0'; // Null-terminate the response

    // Print the loan details received
    printf("%s", response);
}

void employee_module(int socket)
{
    while (1){
        int op_choice;

        employee_menu();

        printf("Enter your choice of Operation:");
        scanf("%d", &op_choice);

        send(socket, &op_choice, sizeof(op_choice), 0);

        switch (op_choice){
        case 1:
            add_customer(socket);
            break;
        case 2:
            modify_customer(socket);
            break;
        case 3:
            process_loan(socket);
            break;
        case 4:
            view_assigned_loans(socket);
            break;
        case 5:
            change_emp_password(socket);
            break;
        case 6:
            return;
        case 7:
            return;
            // case 4:
            //     transfer_funds(socket);
            //     break;
            // case 6:
            //     change_password(socket);
            //     break;
        }
    }
}

#endif