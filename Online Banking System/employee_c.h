#ifndef EMPLOYEE_C_H
#define EMPLOYEE_C_H
#include "common.h"

#define BUFFER_SIZE 1024

void employee_menu(){
    printf("\n\n------------Employee Module:--------------\n");
    printf("1. Add New Custmomer\n");
    printf("2. Modify Customer Details\n");
    printf("3. Process Loan Applications\n");
    printf("4. Approve/Reject Loans\n");
    printf("5. View Assigned Loan Applications\n");
    printf("6. Change Password\n");
    printf("7. Logout\n");
    printf("8. Exit\n\n");
}

void add_customer(int socket){
    char username[50],password[50], response[50];
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

void modify_customer(int socket){
    char username[50],password[50], response[50];
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

void change_emp_password(int socket){
    char new_password[50], response[50];

    printf("\nEnter new password:");
    scanf("%s", new_password);

    send(socket, new_password, sizeof(new_password), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void employee_module(int socket) {
    int op_choice;

    employee_menu();

    printf("Enter your choice of Operation:");
    scanf("%d", &op_choice);
    
    send(socket, &op_choice, sizeof(op_choice), 0);

    switch(op_choice){
        case 1:
            add_customer(socket);
            break;
        case 2:
            modify_customer(socket);
            break;
        case 6:
            change_emp_password(socket);
            break;
        // case 3:
        //     withdraw_money(socket);
        //     break;
        // case 4:
        //     transfer_funds(socket);
        //     break;
        // case 6:
        //     change_password(socket);
        //     break;
    }
}

#endif