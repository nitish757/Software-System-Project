#ifndef LOGIN_H
#define LOGIN_H

int login(int sock) {
    char username[50], password[50], role[50];
    int flag;

    printf("Enter Username: ");
    scanf("%s", username);
    printf("Enter Password: ");
    scanf("%s", password);

    send(sock, username, sizeof(username), 0);
    send(sock, password, sizeof(password), 0);
    
    read(sock, &flag, sizeof(flag));
    return flag;
}

#endif
