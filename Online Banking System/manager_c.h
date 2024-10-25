#ifndef MANAGER_C_H
#define MANAGER_C_H
#include "common.h"

#define BUFFER_SIZE 1024

void manager_menu()
{
    printf("\n\n------------Manager Module:--------------\n");
    printf("1. Activate/Deactivate Customer Accounts\n");
    printf("2. Assign Loan Application Processes to Employees\n");
    printf("3. Review Customer Feedback\n");
    printf("4. Change Password\nn");
    printf("5. Logout\n");
    printf("6. Exit\n\n");
}

void activate_deactivate_customer(int socket)
{
    int active;
    char username[50], response[50];

    printf("Enter 0 to Deactivate and 1 to Activate a customer:");
    scanf("%d", &active);
    send(socket, &active, sizeof(active), 0);

    if (active)
    {
        printf("Enter username to Activate:");
        scanf("%s", username);
        send(socket, username, sizeof(username), 0);
    }
    else
    {
        printf("Enter username to Deactivate:");
        scanf("%s", username);
        send(socket, username, sizeof(username), 0);
    }

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void assign_loan(int client_socket)
{
    int loan_id;
    char employee_username[50];

    printf("Enter Loan ID to assign: ");
    scanf("%d", &loan_id);
    printf("Enter Employee Username to assign: ");
    scanf("%s", employee_username);

    // Send loan ID and employee username to server
    send(client_socket, &loan_id, sizeof(loan_id), 0);
    send(client_socket, employee_username, sizeof(employee_username), 0);

    // Receive confirmation from server
    char response[50];
    recv(client_socket, response, sizeof(response), 0);
    printf("%s\n", response);
}

void change_man_password(int socket)
{
    char new_password[50], response[50];

    printf("\nEnter new password:");
    scanf("%s", new_password);

    send(socket, new_password, sizeof(new_password), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void manager_module(int socket)
{

    while (1){
        int op_choice;

        manager_menu();

        printf("Enter your choice of Operation:");
        scanf("%d", &op_choice);

        send(socket, &op_choice, sizeof(op_choice), 0);

        switch (op_choice){
        case 1:
            activate_deactivate_customer(socket);
            break;
        case 2:
            assign_loan(socket);
            break;
        case 4:
            change_man_password(socket);
            break;
        case 5:
            return;
        case 6:
            // close(socket);
            return;
        }
    }
}
#endif