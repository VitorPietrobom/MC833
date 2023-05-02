#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "settings.h"

#define PORT 8080

const char *OPTIONS[] = {"Cadastrar perfil", "Buscar perfil por email", "Listar perfis por ano", "Listar perfis por habilidade", "Listar perfis por curso", "Listar todos os perfis", "Deletar perfil", "Sair"};

char* cadastrarPerfil() {
    // Array de labels dos campos
    char *fields[] = {"email", "nome", "sobrenome", "cidade", "formação", "ano de formatura", "habilidades"};

    // Objeto JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddNumberToObject(root, "operation", CADASTRAR_PERFIL);

    // Loop para preencher o objeto JSON
    for (int i = 0; i < 7; i++) {
        char inputPerfil[100];
        printf("Digite o %s: ", fields[i]);
        scanf("%[^\n]%*c", &inputPerfil);
        //input[strcspn(input, "\n")] = 0;  // Remove o caractere de nova linha do final da entrada
        cJSON_AddStringToObject(data, fields[i], inputPerfil);
    }

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

char* buscarPerfil() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddNumberToObject(root, "operation", BUSCAR_PERFIL_EMAIL);

    // Preencher requisição como o email
    char input[100];
    printf("Digite o email pelo qual deseja pesquisar: ");
    scanf("%[^\n]%*c", &input);
    cJSON_AddStringToObject(data, "email", input);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

char* listarPerfis() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "operation", LISTAR_PERFIS_COMPLETO);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

char* listarFiltradoCurso() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddNumberToObject(root, "operation", LISTAR_PERFIL_CURSO);

    // Preencher requisição como o email
    char input[100];
    printf("Digite o curso pelo qual deseja filtrar: ");
    scanf("%[^\n]%*c", &input);
    cJSON_AddStringToObject(data, "formação", input);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

char* listarFiltradoAno() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddNumberToObject(root, "operation", LISTAR_PERFIL_ANO);

    // Preencher requisição como o email
    char input[100];
    printf("Digite o ano de formação pelo qual deseja filtrar: ");
    scanf("%[^\n]%*c", &input);
    cJSON_AddStringToObject(data, "ano de formatura", input);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

char* listarFiltradoHabilidades() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddNumberToObject(root, "operation", LISTAR_PERFIL_HABILIDADE);

    // Preencher requisição como o email
    char input[100];
    printf("Digite a habilidade pela qual deseja filtrar: ");
    scanf("%[^\n]%*c", &input);
    cJSON_AddStringToObject(data, "habilidades", input);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

char* removerPerfil() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "data", data);
    cJSON_AddNumberToObject(root, "operation", DELETAR_PERFIL);

    // Preencher requisição como o email
    char input[100];
    printf("Digite o email do perfil que deseja remover: ");
    scanf("%[^\n]%*c", &input);
    cJSON_AddStringToObject(data, "email", input);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);
    printf("JSON gerado: %s\n", json_string);

    return json_string;
}

int chooseOperation(int sock) {
    struct sockaddr_in serv_addr;
    char input[100];
    int option = -1;

    // Get operation ID from user
    printf("Que operação gostaria de realizar?\n");
    for(int i = 0; i < 8; i++) {
        printf("%d. %s\n", i+1, OPTIONS[i]);
    }

    scanf("%[^\n]%*c", &input);

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

    printf("Conectado ao servidor\n");
    // Switch that calls the function according to the user's input
    switch (atoi(input))
    {
    case CADASTRAR_PERFIL:
    {
        option = CADASTRAR_PERFIL;

        char* profile = cadastrarPerfil();
        // Send message to server
        printf("Sending message to server\n%s\n", profile);
        send(sock, profile, strlen(profile), 0);
        break;
    }
    
    case BUSCAR_PERFIL_EMAIL:
        option = BUSCAR_PERFIL_EMAIL;

        char* emailSearch = buscarPerfil();
        // Send message to server
        printf("Sending message to server\n%s\n", emailSearch);
        send(sock, emailSearch, strlen(emailSearch), 0);
        break;

    case LISTAR_PERFIL_ANO:
        option = LISTAR_PERFIL_ANO;

        char* filterAnoRequest = listarFiltradoAno();
        // Send message to server
        printf("Sending message to server\n%s\n", filterAnoRequest);
        send(sock, filterAnoRequest, strlen(filterAnoRequest), 0);
        break;

    case LISTAR_PERFIL_HABILIDADE:
        option = LISTAR_PERFIL_HABILIDADE;

        char* filterAbilitiesRequest = listarFiltradoHabilidades();
        // Send message to server
        printf("Sending message to server\n%s\n", filterAbilitiesRequest);
        send(sock, filterAbilitiesRequest, strlen(filterAbilitiesRequest), 0);
        break;

    case LISTAR_PERFIL_CURSO:
        option = LISTAR_PERFIL_CURSO;

        char* filterRequest = listarFiltradoCurso();
        // Send message to server
        printf("Sending message to server\n%s\n", filterRequest);
        send(sock, filterRequest, strlen(filterRequest), 0);
        break;

    case LISTAR_PERFIS_COMPLETO:
        option = LISTAR_PERFIS_COMPLETO;
        
        // Send message to server
        char* listing = listarPerfis();
        printf("Requesting listing to server\n");
        send(sock, listing, strlen(listing), 0);
        
        break;
    
    case DELETAR_PERFIL:
        option = DELETAR_PERFIL;

        char* request = removerPerfil();
        // Send message to server
        printf("Sending message to server\n%s\n", request);
        send(sock, request, strlen(request), 0);
        break;
    
    case SAIR:
        option = SAIR;
        break;
    
    default:
        printf("Opção inválida\n");
        break;
    }

    return option;
}

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    int option = -1;

    while(option != SAIR) {
        option = chooseOperation(sock);
        printf("%i", option);
  
        // Read data from server
        valread = recv(sock, buffer, 1024, 0);
        printf("Return of valread: %d\n",valread);
        printf("Input: %s\n",buffer);
    }

    return 0;
}