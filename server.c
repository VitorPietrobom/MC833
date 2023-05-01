#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080



// FINALIZADA - FUNCIONANDO  - NÃO ALTERAR
void registrarPerfil(char* nomeArquivo, char* perfilString) {
    // Abrir o arquivo JSON existente ou criar um novo arquivo se ele não existir
    FILE* arquivoJSON;
    cJSON* raiz;
    if ((arquivoJSON = fopen(nomeArquivo, "rb")) == NULL) {
        // Criar um novo objeto JSON vazio se o arquivo não existir
        raiz = cJSON_CreateObject();
    } else {
        // Carregar o objeto JSON existente do arquivo
        fseek(arquivoJSON, 0, SEEK_END);
        long tamanhoArquivo = ftell(arquivoJSON);
        rewind(arquivoJSON);
        char* conteudoArquivo = malloc(tamanhoArquivo + 1);
        fread(conteudoArquivo, 1, tamanhoArquivo, arquivoJSON);
        fclose(arquivoJSON);
        raiz = cJSON_Parse(conteudoArquivo);
        free(conteudoArquivo);
    }

    // Criar um novo objeto JSON para o perfil e adicioná-lo à matriz "perfis"
    cJSON* perfil = cJSON_Parse(perfilString);
    
    cJSON* perfis = cJSON_GetObjectItem(raiz, "perfis");
    if (perfis == NULL) {
        perfis = cJSON_AddArrayToObject(raiz, "perfis");
    }
    cJSON_AddItemToArray(perfis, perfil);


    // Salvar o objeto JSON atualizado no arquivo
    arquivoJSON = fopen(nomeArquivo, "wb");
    char* conteudoJSON = cJSON_Print(raiz);
    printf("%s\n",conteudoJSON);
    fwrite(conteudoJSON, 1, strlen(conteudoJSON), arquivoJSON);
    fclose(arquivoJSON);
    free(conteudoJSON);

    // Liberar a memória alocada pelo objeto JSON e sua matriz de perfis
    cJSON_Delete(raiz);
}

void* handleClient(void* arg) {
    int client_socket = *(int*)arg;

    int read_size;    
    char buffer[1024];
    while (1) {
        bzero(buffer, 1024);
        read_size = recv(client_socket, buffer, sizeof(buffer),0);
        registrarPerfil("perfis.json", buffer);
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    //registrarPerfil("perfis.json", "penis@meu.bola.esquerda", "Boris o Amendoim", "Amendoim", "Gramado serrano", "Mestrado em Amendoim", 2020, "Fazer amendoim e comer amendoim");
    

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int client_socket;

    while (1) {
        client_socket = accept(server_fd, NULL, NULL);
        printf("client accepted\n");
        pthread_t tid; // precisa de thread pra atender em simultaneo
        pthread_create(&tid, NULL, handleClient, (void*)&client_socket);
        pthread_detach(tid);
    }

    close(client_socket);

    // // Accept incoming connection
    // if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }

    // Read data from client
    // valread = recv(new_socket, buffer, 1024, 0);
    // registrarPerfil("perfis.json", buffer);
    //printf("Return of valread: %d\n",valread);
    //printf("Input: %s\n",buffer);

    // // Send message to client
    // send(new_socket, hello, strlen(hello), 0);

    return 0;
}

