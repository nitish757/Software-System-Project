#include "common.h"
#include "login.h"
#include "customer_c.h"
#include "employee_c.h"
#include "manager_c.h"
#include "admin_c.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void module(int socket, int choice)
{
    switch (choice)
    {
    case 1:
        customer_module(socket);
        break;
    case 2:
        employee_module(socket);
        break;
    case 3:
        manager_module(socket);
        break;
    case 4:
        admin_module(socket);
        break;
    default:
        printf("Invalid choice: %d\n", choice);
        break;
    }
}

void menu(int socket)
{
    int choice;
    char buffer[BUFFER_SIZE] = {0};

    while (1)
    {
        // Display the main menu
        printf("\n====== Bank Management System ======\n");
        printf("1. Customer Module\n");
        printf("2. Bank Employee Module\n");
        printf("3. Manager Module\n");
        printf("4. Admin Module\n");
        printf("5. Exit\n");
        printf("Choose a module: ");

        scanf("%d", &choice);

        send(socket, &choice, sizeof(choice), 0);

        if (choice == 5)
        {
            return;
        }

        if (choice >= 1 && choice <= 4){
            if (!login(socket)){
                printf("Login failed. Please try again.\n");
                close(socket);
                continue; 
            }
            else{
                printf("Login successful!\n");
            }

            module(socket, choice);
        }
        else
        {
            printf("Invalid choice. Please select a valid module.\n");
        }
    }
}

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        printf("Invalid address / Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection Failed\n");
        return -1;
    }
    
    menu(sock);

    close(sock);
    return 0;
}

