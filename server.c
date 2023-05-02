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
    int index = 0;
    cJSON_ArrayForEach(perfil, perfis) {
        if (filtro == "habilidades") {
            if (strstr(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, filtro)), campo) != NULL) {
                cJSON_AddItemReferenceToArray(perfisFiltrados, perfil);
            }
        } else {
            if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, filtro)), campo) == 0) {
                cJSON_AddItemReferenceToArray(perfisFiltrados, perfil);
            }
        }
        index++;
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
    int index = 0;
    cJSON_ArrayForEach(perfil, perfis) {
        if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(perfil, "email")), email) == 0) {
            cJSON_AddItemReferenceToArray(perfisFiltrados, perfil);
            break;
        }
        index++;
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
void callOperation(char* buffer, int socket) {
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
        send(socket, filtered_string, strlen(filtered_string), 0);
        break;

    case LISTAR_PERFIL_ANO:
        filtered_string = listarFiltrado(ARQUIVO, buffer, "ano de formatura");
        send(socket, filtered_string, strlen(filtered_string), 0);
        break;

    case LISTAR_PERFIL_CURSO:
        filtered_string = listarFiltrado(ARQUIVO, buffer, "formação");
        send(socket, filtered_string, strlen(filtered_string), 0);
        break;

    case LISTAR_PERFIL_HABILIDADE:
        filtered_string = listarFiltrado(ARQUIVO, buffer, "habilidades");
        send(socket, filtered_string, strlen(filtered_string), 0);
        break;

    case LISTAR_PERFIS_COMPLETO:
        filtered_string = listarPerfis(ARQUIVO);
        send(socket, filtered_string, strlen(filtered_string), 0);
        break;

    case DELETAR_PERFIL:
        removerPerfil(ARQUIVO, buffer);
        break;
    
    default:
        break;
    }
}

void *handle_connection(void *arg) {
    int new_socket = *(int*)arg;
    char buffer[1024] = {0};

    // Lê mensagem do cliente
    int valread = recv(new_socket, buffer, 1024, 0);
    callOperation(buffer, new_socket);

    // Fecha socket
    close(new_socket);

    // Sai da thread
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, opt = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Cria socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configura as opções de socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configura as opções do server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Relaciona socket ao endereço
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Começa a escutar novas conexões
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Aceita conexões
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Cria novas threads
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection, &new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Libera o espaço da thread
        pthread_detach(thread);
    }

    return 0;
}
