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
    printf("%s\n", perfilString);
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

void *handle_connection(void *arg) {
    int new_socket = *(int*)arg;
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // Read data from client
    int valread = recv(new_socket, buffer, 1024, 0);
    registrarPerfil("perfis.json", buffer);

    // Send message to client
    send(new_socket, hello, strlen(hello), 0);

    // Close socket
    close(new_socket);

    // Exit thread
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

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

    while (1) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create new thread to handle connection
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection, &new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Detach thread to free resources when it exits
        pthread_detach(thread);
    }

    return 0;
}

