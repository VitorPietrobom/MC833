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
#define MAXLINE 1024

const char *OPTIONS[] = {"Cadastrar perfil", "Buscar perfil por email", "Listar perfis por ano", "Listar perfis por habilidade", "Listar perfis por curso", "Listar todos os perfis", "Deletar perfil", "Baixar imagem", "Sair"};

// Funcao auxiliar para imprimir um usuario
void printProfile(cJSON* perfil, int index) {
    char* email = cJSON_GetObjectItem(perfil, "email")->valuestring;
    char* nome = cJSON_GetObjectItem(perfil, "nome")->valuestring;
    char* sobrenome = cJSON_GetObjectItem(perfil, "sobrenome")->valuestring;
    char* cidade = cJSON_GetObjectItem(perfil, "cidade")->valuestring;
    char* formacao = cJSON_GetObjectItem(perfil, "formação")->valuestring;
    char* anoFormatura = cJSON_GetObjectItem(perfil, "ano de formatura")->valuestring;
    char* habilidades = cJSON_GetObjectItem(perfil, "habilidades")->valuestring;
    printf("Perfil %d:\n", index + 1);
    printf("\tEmail: %s\n", email);
    printf("\tNome completo: %s %s\n", nome, sobrenome);
    printf("\tCidade: %s\n", cidade);
    printf("\tFormação: %s\n", formacao);
    printf("\tAno de formatura: %s\n", anoFormatura);
    printf("\tHabilidades: %s\n", habilidades);
}

// funcao auxiliar que imprime uma lista de usuarios
void printListProfiles(int sock, char* filtro, struct sockaddr_in server_address) {
    char buffer[MAXLINE];

    int len;

    recvfrom(sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&server_address, &len);
    cJSON* userListJson = cJSON_Parse(buffer);

    printf("\n\n\n----------------------------- Resultado --------------------------------\n\n");

    int numPerfis = cJSON_GetArraySize(userListJson);
    printf("Perfis no arquivo: %d\n\n", numPerfis);
    for (int i = 0; i < numPerfis; i++) {
        cJSON* perfil = cJSON_GetArrayItem(userListJson, i);
        printProfile(perfil, i);
    }
}

// Funcao 1 -  Prepara input para CADASTRAR_PERFIL
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
        cJSON_AddStringToObject(data, fields[i], inputPerfil);
    }

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);

    return json_string;
}

// funcao 2 -  Prepara input para BUSCAR_PERFIL_EMAIL
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


    return json_string;
}

// Funcao 3 - Prepara input para LISTAR_PERFIS_COMPLETO
char* listarPerfis() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "operation", LISTAR_PERFIS_COMPLETO);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);

    return json_string;
}

// Funcao 4 -  Prepara input para LISTAR_PERFIL_CURSO
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

    return json_string;
}

// Funcao 5 -  Prepara input para LISTAR_PERFIL_ANO
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

    return json_string;
}

// Funcao 6 -  Prepara input para LISTAR_PERFIL_HABILIDADE
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

    return json_string;
}

// Funcao 7 -  Prepara input para DELETAR_PERFIL
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

    return json_string;
}

// Funcao 8 - Prepara input para DOWNLOAD_IMAGEM
char* downloadImagem() {
    // Objeto JSON
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "operation", DOWNLOAD_IMAGEM);

    // Converte o objeto JSON para uma string formatada
    char *json_string = cJSON_Print(root);

    return json_string;
}


// Funcao utilizada para preparar a escolha do cliente e enviar para o usuario
int chooseOperation(int sock) {
    struct sockaddr_in serv_addr;
    char input[100];
    int option = -1;

    // Pega o ID da operacao
    printf("Que operação gostaria de realizar?\n");
    for(int i = 0; i < 9; i++) {
        printf("%d. %s\n", i+1, OPTIONS[i]);
    }

    scanf("%[^\n]%*c", &input);

    

    memset(&serv_addr, 0, sizeof(serv_addr));

    // Configura o endereco do server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Switch que ajuda na chamada das funcoes
    switch (atoi(input))
    {
    case CADASTRAR_PERFIL:
    {
        option = CADASTRAR_PERFIL;

        char* profile = cadastrarPerfil();
        // Envia mensagem para o servidor
        printf("Enviando pedido de cadastro para o servidor\n");
        sendto(sock, (const char *)profile, strlen(profile), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        break;
    }
    
    case BUSCAR_PERFIL_EMAIL:
        option = BUSCAR_PERFIL_EMAIL;

        char* emailSearch = buscarPerfil();
        // Envia mensagem para o servidor
        printf("Enviando pedido de busca por email para o servidor\n");
        sendto(sock, (const char *)emailSearch, strlen(emailSearch), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        printListProfiles(sock, "", serv_addr);
        break;

    case LISTAR_PERFIL_ANO:
        option = LISTAR_PERFIL_ANO;

        char* filterAnoRequest = listarFiltradoAno();
        // Envia mensagem para o servidor
        printf("Enviando pedido de listagem por ano de formacao para o servidor\n");
        sendto(sock, (const char *)filterAnoRequest, strlen(filterAnoRequest), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        printListProfiles(sock, "ano de formatura", serv_addr);
        break;

    case LISTAR_PERFIL_HABILIDADE:
        option = LISTAR_PERFIL_HABILIDADE;

        char* filterAbilitiesRequest = listarFiltradoHabilidades();
        // Envia mensagem para o servidor
        printf("Enviando pedido de listagem por habilidade para o servidor\n");
        sendto(sock, (const char *)filterAbilitiesRequest, strlen(filterAbilitiesRequest), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        printListProfiles(sock, "habilidades", serv_addr);
        break;

    case LISTAR_PERFIL_CURSO:
        option = LISTAR_PERFIL_CURSO;

        char* filterRequest = listarFiltradoCurso();
        // Envia mensagem para o servidor
        printf("Enviando pedido de listagem por curso para o servidor\n");
        sendto(sock, (const char *)filterRequest, strlen(filterRequest), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        printListProfiles(sock, "formação", serv_addr);
        break;

    case LISTAR_PERFIS_COMPLETO:
        option = LISTAR_PERFIS_COMPLETO;
        
        // Envia mensagem para o servidor
        char* listing = listarPerfis();
        printf("Enviando pedido de listagem para o servidor\n");
        sendto(sock, (const char *)listing, strlen(listing), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        printListProfiles(sock, "", serv_addr);
        break;
    
    case DELETAR_PERFIL:
        option = DELETAR_PERFIL;

        char* downloadRequest = removerPerfil();
        // Envia mensagem para o servidor
        printf("Enviando pedido de remocao de usuario para o servidor\n");
        sendto(sock, (const char *)downloadRequest, strlen(downloadRequest), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
        break;

    case DOWNLOAD_IMAGEM:
        option = DOWNLOAD_IMAGEM;

        char* request = downloadImagem();
        // Envia mensagem para o servidor
        printf("Enviando pedido de download de imagem para o servidor\n");
        sendto(sock, (const char *)request, strlen(request), 0, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
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
    int sock;
    int option = -1;

    

    // Roda ate o usuario escolher sair
    while(option != SAIR) {

        // Cria um socket file descriptor
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            exit(EXIT_FAILURE);
        }

        option = chooseOperation(sock);

        printf("\n\n\n\n\n\n\n");

        close(sock);
    }

    return 0;
}