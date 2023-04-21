#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include <unistd.h>
#include <string.h>

#define PORT 8080


char* formatJson() {
    // Array de labels dos campos
    char *fields[] = {"email", "nome", "sobrenome", "cidade", "formação", "ano de formatura", "habilidades"};

    // Objeto JSON
    cJSON *root = cJSON_CreateObject();

    // Loop para preencher o objeto JSON
    for (int i = 0; i < 7; i++) {
        char input[100];
        printf("Digite o %s: ", fields[i]);
        // a scanf that reads until the enter key is pressed
        scanf("%[^\n]%*c", &input);
        //input[strcspn(input, "\n")] = 0;  // Remove o caractere de nova linha do final da entrada
        cJSON_AddStringToObject(root, fields[i], input);
    }

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};

    char* profile = formatJson();

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Set server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Send message to server
    send(sock, profile, strlen(profile), 0);

    // Read data from server
    valread = recv(sock, buffer, 1024, 0);
    printf("Return of valread: %d\n",valread);
    printf("Input: %s\n",buffer);

    return 0;
}