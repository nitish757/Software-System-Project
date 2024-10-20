#ifndef ADMIN_C_H
#define ADMIN_C_H
#include "common.h"

#define BUFFER_SIZE 1024

void admin_menu(){
    printf("\n\n------------Admin Module:--------------\n");
    printf("1. Add New Bank Employee\n");
    printf("2. Modify Customer/Employee Details\n");
    printf("3. Manage User Roles\n");
    printf("4. Change Password\n");
    printf("5. Logout\n");
    printf("6. Exit\n\n");
}

void add_employee(int socket){
    char username[50],password[50], response[50];

    printf("\nEnter username of new employee:");
    scanf("%s", username);
    send(socket, username, sizeof(username), 0);

    printf("\nEnter password of new employee:");
    scanf("%s", password);
    send(socket, password, sizeof(password), 0);

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void modify_employee(int socket){
    char username[50],password[50], response[50];
    int salary, choice;

    printf("\nEnter 0 for customer and 1 for Employee:");
    scanf("%d", &choice);
    send(socket, &choice, sizeof(choice), 0);

    if(choice){
        printf("\nEnter username of employee:");
        scanf("%s", username);
        send(socket, username, sizeof(username), 0);

        printf("\nEnter new password of employee:");
        scanf("%s", password);
        send(socket, password, sizeof(password), 0);

        printf("\nEnter new salary of employee:");
        scanf("%d", &salary);
        send(socket, &salary, sizeof(salary), 0);
    }
    else{
        printf("\nEnter username of customer:");
        scanf("%s", username);
        send(socket, username, sizeof(username), 0);

        printf("\nEnter new password of customer:");
        scanf("%s", password);
        send(socket, password, sizeof(password), 0);

        printf("\nEnter new balance of customer:");
        scanf("%d", &salary);
        send(socket, &salary, sizeof(salary), 0);
    }
    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void admin_module(int socket){
    int op_choice;

    admin_menu();

    printf("Enter your choice of Operation:");
    scanf("%d", &op_choice);
    
    send(socket, &op_choice, sizeof(op_choice), 0);

    switch(op_choice){
        case 1:
            add_employee(socket);
            break;
        case 2:
            modify_employee(socket);
            break;
        // case 6:
        //     change_emp_password(socket);
        //     break;
    }
}
#endif