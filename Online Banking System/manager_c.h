#ifndef MANAGER_C_H
#define MANAGER_C_H
#include "common.h"

#define BUFFER_SIZE 1024

void manager_menu(){
    printf("\n\n------------Manager Module:--------------\n");
    printf("1. Activate/Deactivate Customer Accounts\n");
    printf("2. Assign Loan Application Processes to Employees\n");
    printf("3. Review Customer Feedback\n");
    printf("4. Change Password\nn");
    printf("5. Logout\n");
    printf("6. Exit\n\n");
}

void activate_deactivate_customer(int socket){
    int active;
    char username[50], response[50];

    printf("Enter 0 to Deactivate and 1 to Activate a customer:");
    scanf("%d", &active);
    send(socket, &active, sizeof(active), 0);

    if(active){
        printf("Enter username to Activate:");
        scanf("%s", username);
        send(socket, username, sizeof(username), 0);
    }
    else{
        printf("Enter username to Deactivate:");
        scanf("%s", username);
        send(socket, username, sizeof(username), 0);
    }

    read(socket, response, sizeof(response));
    printf("\n%s", response);
}

void manager_module(int socket){
    int op_choice;

    manager_menu();

    printf("Enter your choice of Operation:");
    scanf("%d", &op_choice);
    
    send(socket, &op_choice, sizeof(op_choice), 0);

    switch(op_choice){
        case 1:
            activate_deactivate_customer(socket);
            break;
        // case 2:
        //     modify_customer(socket);
        //     break;
        // case 6:
        //     change_emp_password(socket);
        //     break;
    }
}
#endif