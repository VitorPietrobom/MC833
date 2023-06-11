#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "settings.h"

#define PORT 8080
#define MAXLINE 1024
#define ARQUIVO "perfis.json"

// Funcao auxiliar para abrir o .json
void openFile(char* nomeArquivo, FILE** arquivoJSON, cJSON** raiz) {
    if ((*arquivoJSON = fopen(nomeArquivo, "rb")) == NULL) {
        // Criar um novo objeto JSON vazio se o arquivo não existir
        *raiz = cJSON_CreateObject();
    } else {
        // Carregar o objeto JSON existente do arquivo
        fseek(*arquivoJSON, 0, SEEK_END);
        long tamanhoArquivo = ftell(*arquivoJSON);
        rewind(*arquivoJSON);
        char* conteudoArquivo = malloc(tamanhoArquivo + 1);
        fread(conteudoArquivo, 1, tamanhoArquivo, *arquivoJSON);
        fclose(*arquivoJSON);
        *raiz = cJSON_Parse(conteudoArquivo);
        free(conteudoArquivo);
    }
}

void registrarPerfil(char* nomeArquivo, char* perfilString) {
    // Abrir o arquivo JSON existente ou criar um novo arquivo se ele não existir
    FILE* arquivoJSON;
    cJSON* raiz;
    openFile(nomeArquivo, &arquivoJSON, &raiz);

    // Criar um novo objeto JSON para o perfil e adicioná-lo à matriz "perfis"
    cJSON* jsonRequest = cJSON_Parse(perfilString);
    cJSON* perfil = cJSON_GetObjectItem(jsonRequest, "data");
    
    cJSON* perfis = cJSON_GetObjectItem(raiz, "perfis");
    if (perfis == NULL) {
        perfis = cJSON_AddArrayToObject(raiz, "perfis");
    }
    
    cJSON_AddItemToArray(perfis, perfil);

    // Remove os 1's do meio do array
    if (perfis != NULL) {
        int i = 0;
        while (cJSON_GetArrayItem(perfis, i) != NULL) {
            cJSON* item = cJSON_GetArrayItem(perfis, i);
            if (item->type == cJSON_Number && item->valueint == 1) {
                cJSON_DeleteItemFromArray(perfis, i);
            } else {
                i++;
            }
        }
    }

    // Salvar o objeto JSON atualizado no arquivo
    arquivoJSON = fopen(nomeArquivo, "wb");
    char* conteudoJSON = cJSON_Print(raiz);
    printf("Usuário adicionado.\nConteúdo:\n%s\n",conteudoJSON);
    fwrite(conteudoJSON, 1, strlen(conteudoJSON), arquivoJSON);
    fclose(arquivoJSON);
    free(conteudoJSON);

    // Liberar a memória alocada pelo objeto JSON e sua matriz de perfis
    cJSON_Delete(raiz);
}

char* listarPerfis(char* nomeArquivo) {
    // Abrir o arquivo JSON existente ou criar um novo arquivo se ele não existir
    FILE* arquivoJSON;
    cJSON* raiz;
    openFile(nomeArquivo, &arquivoJSON, &raiz);

    // Listar todos os perfis contidos no arquivo JSON
    cJSON* perfis = cJSON_GetObjectItem(raiz, "perfis");
    if (perfis == NULL) {
        printf("Não há perfis no arquivo %s\n", nomeArquivo);
        cJSON_Delete(raiz);
        return "";
    }

    char *json_string = cJSON_Print(perfis);

    // Liberar a memória alocada pelo objeto JSON e sua matriz de perfis
    cJSON_Delete(raiz);

    return json_string;
}

char* listarFiltrado(char* nomeArquivo, char* stringRequest, char* filtro) {
    // Abrir o arquivo JSON existente ou criar um novo arquivo se ele não existir
    FILE* arquivoJSON;
    cJSON* raiz;
    openFile(nomeArquivo, &arquivoJSON, &raiz);

    // Extrair o valor a ser filtrado
    cJSON* jsonRequest = cJSON_Parse(stringRequest);
    cJSON* data = cJSON_GetObjectItem(jsonRequest, "data");
    char* campo = cJSON_GetStringValue(cJSON_GetObjectItem(data, filtro));

    // Percorrer a matriz de perfis e filtra os perfis com a formação
    cJSON* perfisFiltrados = cJSON_CreateObject();
    cJSON* perfis = cJSON_GetObjectItem(raiz, "perfis");
    cJSON* perfil;
    cJSON_ArrayForEach(perfil, perfis) {
        if (strcmp(filtro, "habilidades") == 0) {
            if (strstr(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, filtro)), campo) != NULL) {
                cJSON_AddItemReferenceToArray(perfisFiltrados, perfil);
            }
        } else {
            if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, filtro)), campo) == 0) {
                cJSON_AddItemReferenceToArray(perfisFiltrados, perfil);
            }
        }
    }

    char *json_string = cJSON_Print(perfisFiltrados);

    // Liberar a memória alocada pelo objeto JSON e sua matriz de perfis
    cJSON_Delete(perfisFiltrados);
    cJSON_Delete(raiz);
    return json_string;
}

char* buscarPerfil(char* nomeArquivo, char* stringRequest) {
    // Abrir o arquivo JSON existente ou criar um novo arquivo se ele não existir
    FILE* arquivoJSON;
    cJSON* raiz;
    openFile(nomeArquivo, &arquivoJSON, &raiz);

    // Extrair o email do usuário a ser removido
    cJSON* jsonRequest = cJSON_Parse(stringRequest);
    cJSON* data = cJSON_GetObjectItem(jsonRequest, "data");
    char* email = cJSON_GetStringValue(cJSON_GetObjectItem(data, "email"));

    // Percorrer a matriz de perfis e remover o perfil com o email correspondente
    cJSON* perfis = cJSON_GetObjectItem(raiz, "perfis");
    cJSON* perfisFiltrados = cJSON_CreateObject();
    cJSON* perfil;
    cJSON_ArrayForEach(perfil, perfis) {
        if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, "email")), email) == 0) {
            cJSON_AddItemReferenceToArray(perfisFiltrados, perfil);
            break;
        }
    }

    char *json_string = cJSON_Print(perfisFiltrados);

    // Liberar a memória alocada pelo objeto JSON e sua matriz de perfis
    cJSON_Delete(raiz);
    return json_string;
}

void removerPerfil(char* nomeArquivo, char* stringRequest) {
    // Abrir o arquivo JSON existente ou criar um novo arquivo se ele não existir
    FILE* arquivoJSON;
    cJSON* raiz;
    openFile(nomeArquivo, &arquivoJSON, &raiz);

    // Extrair o email do usuário a ser removido
    cJSON* jsonRequest = cJSON_Parse(stringRequest);
    cJSON* data = cJSON_GetObjectItem(jsonRequest, "data");
    char* email = cJSON_GetStringValue(cJSON_GetObjectItem(data, "email"));

    // Percorrer a matriz de perfis e remover o perfil com o email correspondente
    cJSON* perfis = cJSON_GetObjectItem(raiz, "perfis");
    cJSON* perfil;
    int index = 0;
    cJSON_ArrayForEach(perfil, perfis) {
        if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, "email")), email) == 0) {
            cJSON_DeleteItemFromArray(perfis, index);
            break;
        }
        index++;
    }

    // Atualizar a matriz de perfis no objeto JSON
    cJSON_ReplaceItemInObject(raiz, "perfis", perfis);

    // Salvar o objeto JSON atualizado no arquivo
    arquivoJSON = fopen(nomeArquivo, "wb");
    char* conteudoJSON = cJSON_Print(raiz);
    printf("Usuário removido.\nConteúdo:\n%s\n",conteudoJSON);
    fwrite(conteudoJSON, 1, strlen(conteudoJSON), arquivoJSON);
    fclose(arquivoJSON);
    free(conteudoJSON);

    // Liberar a memória alocada pelo objeto JSON e sua matriz de perfis
    cJSON_Delete(raiz);
}

// Função utilizada para chamar as operações e enviar informações para o cliente
void callOperation(char* buffer, int socket, struct sockaddr* client_addr, socklen_t client_addr_len) {
    cJSON* jsonRequest = cJSON_Parse(buffer);
    char* filtered_string;

    int operation = cJSON_GetNumberValue(cJSON_GetObjectItem(jsonRequest, "operation"));

    switch (operation)
    {
    case CADASTRAR_PERFIL:
        registrarPerfil(ARQUIVO, buffer);
        break;

    case BUSCAR_PERFIL_EMAIL:
        filtered_string = buscarPerfil(ARQUIVO, buffer);
        sendto(socket, (const char *)filtered_string, strlen(filtered_string), 0, (const struct sockaddr*) client_addr, client_addr_len);
        break;

    case LISTAR_PERFIL_ANO:
        filtered_string = listarFiltrado(ARQUIVO, buffer, "ano de formatura");
        sendto(socket, (const char *)filtered_string, strlen(filtered_string), 0, (const struct sockaddr*) client_addr, client_addr_len);
        break;

    case LISTAR_PERFIL_CURSO:
        filtered_string = listarFiltrado(ARQUIVO, buffer, "formação");
        sendto(socket, (const char *)filtered_string, strlen(filtered_string), 0, (const struct sockaddr*) client_addr, client_addr_len);
        break;

    case LISTAR_PERFIL_HABILIDADE:
        filtered_string = listarFiltrado(ARQUIVO, buffer, "habilidades");
        sendto(socket, (const char *)filtered_string, strlen(filtered_string), 0, (const struct sockaddr*) client_addr, client_addr_len);
        break;

    case LISTAR_PERFIS_COMPLETO:
        filtered_string = listarPerfis(ARQUIVO);
        sendto(socket, (const char *)filtered_string, strlen(filtered_string), 0, (const struct sockaddr*) client_addr, client_addr_len);
        break;

    case DELETAR_PERFIL:
        removerPerfil(ARQUIVO, buffer);
        break;

    case DOWNLOAD_IMAGEM:
        filtered_string = "";
        sendto(socket, (const char *)filtered_string, strlen(filtered_string), 0, (const struct sockaddr*) client_addr, client_addr_len);
        break;
    
    default:
        printf("Operacao invalida\n");
        break;
    }
}

void handle_connection(int socket_fd) {
    char buffer[MAXLINE];
    struct sockaddr_in client_address;
    

    memset(&client_address, 0, sizeof(client_address));

    int client_address_len = sizeof(client_address);

    int n;

    n = recvfrom(socket_fd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&client_address, &client_address_len);
    buffer[n] = '\0';

    // Processa a operação com base nos dados recebidos
    callOperation(buffer, socket_fd, (struct sockaddr *)&client_address, client_address_len);

    // Fecha o socket (se necessário)
    close(socket_fd);
}

int main(int argc, char const *argv[]) {
    while(1) {
        int socket_fd;
        struct sockaddr_in server_addess;
        int addrlen = sizeof(server_addess);

        // Cria socket file descriptor
        if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Configura as opções de socket
        memset(&server_addess, 0, sizeof(server_addess));

        // Configura as opções do servidor
        server_addess.sin_family = AF_INET;
        server_addess.sin_addr.s_addr = INADDR_ANY;
        server_addess.sin_port = htons(PORT);

        // Relaciona o socket ao endereço
        if (bind(socket_fd, (const struct sockaddr *)&server_addess, sizeof(server_addess)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Gera conexão
        handle_connection(socket_fd);
    }

    return 0;
}
