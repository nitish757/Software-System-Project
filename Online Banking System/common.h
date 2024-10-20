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
};

struct Employee{
    char username[50]; 
    char password[50];
    int salary;
};

struct Manager{
    char username[50]; 
    char password[50];
};
#endif