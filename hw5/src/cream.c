#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "csapp.h"
#include <unistd.h>
#include <sys/socket.h>

bool isOnlyDigits(char *digit);
void parseline(int argc, char *argv[]);
void *serviceHandler();

int num_workers, max_entries;
char *port;
queue_t *service_queue;

int main(int argc, char *argv[]) {

    int serverfd, clientfd;

    /*PARSE THE COMMAND LINE TO GET ARGUMENTS*/
    parseline(argc, argv);

    /*INITIALIZE THE SERVICE QUEUE*/
    service_queue = create_queue();

    serverfd = open_listenfd(port);

    pthread_t thread_ids[num_workers];

    /*LISTEN INFINETLY FOR CONNECTIONS*/
    while(1){

        for(int i=0; i < num_workers; i++){

            clientfd = Accept(serverfd, NULL, NULL);
            enqueue(service_queue, (void*)&clientfd);
            printf("Connected to client %d\n",clientfd);
            pthread_create(&thread_ids[i], NULL, serviceHandler, (void *)&clientfd);
        }
    }

    Close(serverfd);

    exit(0);
}

void *serviceHandler(){
    printf("running\n");
    return NULL;
}

void parseline(int argc, char *argv[]){

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

    printf("num_workers: %d\n", num_workers);
    printf("port: %s\n", port);
    printf("max_entries %d\n",max_entries );

    return;
}

bool isOnlyDigits(char digit[]){

    for(int i=0; i< strlen(digit); i++){

        if(!isdigit(digit[i]))
            return false;
    }
    return true;
}


