#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "csapp.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>

bool isOnlyDigits(char *digit);
void parseline(int argc, char *argv[]);
void destroy_function(map_key_t key, map_val_t val);
void *start(void* s);
void *serviceHandler(int clientfd);
void handle_put(int clientfd, request_header_t request_header);
void handle_get(int clientfd, request_header_t request_header);
void handle_evict(int clientfd, request_header_t request_header);
void handle_clear();

int num_workers, max_entries;
char *port;
queue_t *service_queue;
hashmap_t *hashmap;

int main(int argc, char *argv[]) {

    int serverfd, clientfd;

    /*PARSE THE COMMAND LINE TO GET ARGUMENTS*/
    parseline(argc, argv);

    /*INITIALIZE THE SERVICE QUEUE*/
    service_queue = create_queue();

    /*INITIALIZE THE HASH MAP*/
    hashmap = create_map(max_entries, jenkins_one_at_a_time_hash, destroy_function);

    /*SPAWN THE THREADS*/
    pthread_t thread_ids[num_workers];

    for(int i=0; i< num_workers; i++){
        if(pthread_create(&thread_ids[i], NULL, start, NULL) != 0)
                exit(EXIT_FAILURE);
    }

    serverfd = open_listenfd(port);


    /*LISTEN INFINETLY FOR CONNECTIONS*/
    while(1){

            clientfd = Accept(serverfd, NULL, NULL);
            printf("clientfd(main): %d\n", clientfd);

            int *fd = calloc(1, sizeof(clientfd));
            *fd = clientfd;

            enqueue(service_queue, fd);

    }

    Close(serverfd);

    exit(0);
}

/*void close_clientfd(int *clientfd){

    Close(*clientfd);
    free(clientfd);
}*/

void *start(void* s){

    while(1){

        //printf("clientfd(start): %d\n", clientfd);
        int *clientfd = ((int*)dequeue(service_queue));
        int fd = *clientfd;
        free(clientfd);
        serviceHandler(fd);
    }

    return NULL;

}

void *serviceHandler(int clientfd){


    request_header_t request_header;
    response_header_t response_header;

    printf("service: %d %d\n",clientfd, fcntl(clientfd, F_GETFD));

    Rio_readn(clientfd, &request_header, sizeof(request_header));
    if(request_header.request_code == PUT){
        if(request_header.key_size < MIN_KEY_SIZE || request_header.key_size > MAX_KEY_SIZE || request_header.value_size < MIN_VALUE_SIZE || request_header.value_size > MAX_VALUE_SIZE)
            goto bad;
        //KEY AND VALUE ARE VALID
        handle_put(clientfd, request_header);
        Close(clientfd);
        return NULL;
    }

    if(request_header.request_code == GET){
        if(request_header.key_size < MIN_KEY_SIZE || request_header.key_size > MAX_KEY_SIZE)
            goto bad;

        //KEY IS VALID/
        printf("now\n");
        handle_get(clientfd, request_header);
        Close(clientfd);
        return NULL;
    }

    if(request_header.request_code == EVICT){
        if(request_header.key_size < MIN_KEY_SIZE || request_header.key_size > MAX_KEY_SIZE)
            goto bad;

        //KEY IS VALID/
        handle_evict(clientfd, request_header);
        Close(clientfd);
        return NULL;
    }

    if(request_header.request_code == CLEAR){
        handle_clear(clientfd);
        Close(clientfd);
        return NULL;
    }


    bad:
    /*BAD REQUEST*/

    response_header.response_code = UNSUPPORTED;
    response_header.value_size = 0;

    /*WRITE TO THE SOCKET*/
    Rio_writen(clientfd, &response_header, sizeof(response_header));
    Close(clientfd);

    return NULL;

}

void handle_put(int clientfd, request_header_t request_header){


    char *key_base = calloc(1, request_header.key_size);
    char *value_base = calloc(1, request_header.value_size);

    memset(key_base, 0, request_header.key_size);
    memset(value_base, 0, request_header.value_size);

    printf("put %d\n", clientfd);
    Rio_readn(clientfd, key_base, request_header.key_size);
    Rio_readn(clientfd, value_base, request_header.value_size);


    map_key_t key;
    key.key_base = key_base;
    key.key_len = request_header.key_size;

    map_val_t val;
    val.val_base = value_base;
    val.val_len = request_header.value_size;
    //printf("size: %d\n", request_header.value_size);

    /*PUT THE KEY AND VALUE IN THE MAP*/
    bool flag = put(hashmap, key, val, false);

    //printf("keyyyyyyyyyy: %s \n", (char*)((hashmap->nodes+10)->key).key_base);

    /*MAKE THE RESPONSE HEADER*/
    response_header_t response_header;
    if(flag == true){
        response_header.response_code = OK;
        response_header.value_size = request_header.value_size;
    }
    else{
        response_header.response_code = BAD_REQUEST;
        response_header.value_size = 0;
    }

    /*WRITE TO THE SOCKET*/
    Rio_writen(clientfd, &response_header, sizeof(response_header));
    /*WE DON'T NEED TO FREE KEY_BASE AND VAUE_BASE; KEY AND VALUE ARE BEING PUT IN THE MAP*/
    printf("ret exit\n");
    return;
}

void handle_get(int clientfd, request_header_t request_header){

    char *key_base = calloc(1, request_header.key_size);
    char *value_base = calloc(1, request_header.value_size); //DON'T USE THIS VALUE

    memset(key_base, 0, request_header.key_size);
    memset(value_base, 0, request_header.value_size);

    printf("get: %d\n", clientfd);
    Rio_readn(clientfd, key_base, request_header.key_size);
    Rio_readn(clientfd, value_base, request_header.value_size);

    map_key_t key;
    key.key_base = key_base;
    key.key_len = request_header.key_size;

    map_val_t value = get(hashmap, key);

    /*MAKE THE RESPONSE HEADER*/
    response_header_t response_header;
    if(value.val_base == NULL){
        response_header.response_code = NOT_FOUND;
        response_header.value_size = 0;
    }
    else{

        response_header.response_code = OK;
        response_header.value_size = value.val_len;
    }

    /*WRITE TO THE SOCKET*/
    Rio_writen(clientfd, &response_header, sizeof(response_header));
    Rio_writen(clientfd, value.val_base, response_header.value_size);

    /*NEED TO FREE KEY_BASE AND VALUE_BASE*/
    free(key_base);
    free(value_base);

    return;
}

void handle_evict(int clientfd, request_header_t request_header){

    char *key_base = calloc(1, request_header.key_size);
    char *value_base = calloc(1, request_header.value_size); //DON'T USE THIS VALUE

    memset(key_base, 0, request_header.key_size);
    memset(value_base, 0, request_header.value_size);

    Rio_readn(clientfd, key_base, request_header.key_size);
    Rio_readn(clientfd, value_base, request_header.value_size);

    map_key_t key;
    key.key_base = key_base;
    key.key_len = request_header.key_size;

    //map_node_t node;
    delete(hashmap, key);

    //hashmap->destroy_function(node.key, node.val);

    /*MAKE THE RESPONSE HEADER*/
    response_header_t response_header;

    response_header.response_code = OK;
    response_header.value_size = 0;

    /*WRITE TO THE SOCKET*/
    Rio_writen(clientfd, &response_header, sizeof(response_header));

    /*NEED TO FREE KEY_BASE AND VALUE_BASE*/
    free(key_base);
    free(value_base);

    return;

}

void handle_clear(int clientfd){

    bool flag = clear_map(hashmap);

    /*MAKE THE RESPONSE HEADER*/
    response_header_t response_header;
    if(flag == true){
        response_header.response_code = OK;
        response_header.value_size = 0;
    }
    else{
        response_header.response_code = BAD_REQUEST;
        response_header.value_size = 0;
    }

    /*WRITE TO THE SOCKET*/
    Rio_writen(clientfd, &response_header, sizeof(response_header));

    return;
}

void destroy_function(map_key_t key, map_val_t val){

    free(key.key_base);
    key.key_len = 0;
    free(val.val_base);
    val.val_len = 0;

}


void parseline(int argc, char *argv[]){


    if(argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("-h                  Displays this help menu and returns EXIT_SUCCESS.\nNUM_WORKERS         The number of worker threads used to service requests.\nPORT_NUMBER         Port number to listen on for incoming connections.\nMAX_ENTRIES         The maximum number of entries that can be stored in cream's underlying data store.\n");
            exit(EXIT_SUCCESS);
        }

    }

    if(argc < 4 || argc > 5){
        printf("Too many or too few arguments\n");
        exit(EXIT_FAILURE);
    }

    int hflag = -1;
    for(int i=0; i< argc; i++){

        if(strcmp(argv[i], "-h") == 0){
            hflag = i;
            printf("-h                  Displays this help menu and returns EXIT_SUCCESS.\nNUM_WORKERS         The number of worker threads used to service requests.\nPORT_NUMBER         Port number to listen on for incoming connections.\nMAX_ENTRIES         The maximum number of entries that can be stored in cream's underlying data store.\n");
        }

    }

    if(hflag < 0 && argc == 5){
        printf("Unknown flag provided\n");
        exit(EXIT_FAILURE);
    }

    if((hflag < 0 && argc == 4) || (hflag > -1 && argc == 5)){
        int j =0;
        for(int i = 1; i< argc; i++){
            if(isOnlyDigits(argv[i]) == true &&  j == 0){
                num_workers = atoi(argv[i]);
                j++;
                continue;
            }

            if(isOnlyDigits(argv[i]) == true && j == 1){
                port = argv[i];
                j++;
                continue;
            }

            if(isOnlyDigits(argv[i]) == true && j == 2){
                max_entries = atoi(argv[i]);
                j++;
                continue;
            }
        }

        if(j != 3){
            printf("Too many or too few arguments\n");
            exit(EXIT_FAILURE);
        }

    }



    if(hflag > -1 && argc < 5){
         printf("Too many or too few arguments\n");
         exit(EXIT_FAILURE);
    }

    /*printf("num_workers: %d\n", num_workers);
    printf("port: %s\n", port);
    printf("max_entries %d\n",max_entries );*/

    return;
}

bool isOnlyDigits(char digit[]){

    for(int i=0; i< strlen(digit); i++){

        if(!isdigit(digit[i]))
            return false;
    }
    return true;
}


