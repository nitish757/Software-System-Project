#ifndef CUSTOMER_H
#define CUSTOMER_H

#define BUFFER_SIZE 1024

void customer(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE] = "Customer Module:\n 
                                1. View Balance\n
                                2. Deposit Money\n
                                3. Withdraw Money\n
                                4. Transfer Fund\n
                                5. Apply for a Loan\n
                                6. Change Password\n
                                7. Adding Feedback\n
                                8. View Transaction History\n
                                9. Logout\n
                                10. Exit\n";

    send(client_socket, buffer, strlen(buffer), 0);  // Send menu to client

    // Receive customer choice and handle accordingly
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, &choice, sizeof(op_choice));

    switch (choice) {
        // case 1:
        //     send(client_socket, "Balance: $1000\n", 15, 0);
        //     break;
        // case 2:
        //     // Handle Deposit
        //     send(client_socket, "Deposit Success\n", 16, 0);
        //     break;
        // case 3:
        //     // Handle Withdraw
        //     send(client_socket, "Withdraw Success\n", 17, 0);
        //     break;
        // case 4:
        //     // Exit
        //     send(client_socket, "Exiting Customer Module...\n", 27, 0);
        //     return;
        default:
            send(client_socket, "Invalid Choice\n", 15, 0);
    }
}
#endif