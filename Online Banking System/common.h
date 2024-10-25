#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h> 
#include <fcntl.h>
#include <sys/file.h>

struct Customer{
    char username[50]; 
    char password[50];
    int balance;
}customer, employee;

// struct Employee{
//     char username[50]; 
//     char password[50];
//     int salary;
// };

struct Manager{
    char username[50]; 
    char password[50];
}manager, admin;

int generate_loan_id() {
    int loan_id;
    int file = open("loan_id_counter.txt", O_RDWR | O_CREAT, 0644);

    if (file < 0) {
        perror("Error opening loan ID counter file");
        exit(1);
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if (fcntl(file, F_SETLKW, &lock) == -1) {
        perror("Failed to lock the file");
        close(file);
        exit(1);
    }

    if (read(file, &loan_id, sizeof(loan_id)) > 0) {
        loan_id++;
    } else {
        loan_id = 1;  
    }

    lseek(file, 0, SEEK_SET);

    if (write(file, &loan_id, sizeof(loan_id)) < 0) {
        perror("Error writing loan ID to file");
        close(file);
        exit(1);
    }

    lock.l_type = F_UNLCK;
    if (fcntl(file, F_SETLK, &lock) == -1) {
        perror("Failed to unlock the file");
    }

    close(file);
    return loan_id; 
}

#endif