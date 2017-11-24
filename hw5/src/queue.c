#include "queue.h"
#include "csapp.h"

queue_t *create_queue(void) {

    queue_t *queue;

    if((queue = (queue_t*)calloc(sizeof(queue_t), sizeof(queue_t))) == NULL)
        return NULL;

    queue->front = queue->rear = NULL;

    if(sem_init(&queue->items, 0, 0)<0)
        return NULL;

    if(pthread_mutex_init(&queue->lock, NULL) != 0)
        return NULL;

    queue->invalid = false;

    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {

    /*INVALID PARAMETERS*/
    if(self == NULL || destroy_function == NULL){

        errno = EINVAL;
        return false;
    }

    if(pthread_mutex_lock(&self->lock) != 0) /*LOCK THE BUFFER*/
        return false;

    queue_node_t *node1 = self->front;
    queue_node_t *node2;
    while(node1 != NULL){

        destroy_function(node1->item); /*CLEAN UP EVERY ITEM IN THE QUEUE*/
        node2 = node1->next;
        free(node1);
        node1 = node2;
    }

    if(pthread_mutex_unlock(&self->lock) != 0) /*UNLOCK THE BUFFER*/
        return false;

    self->invalid = true;/*SETTING THE INVALID FLAG TO TRUE*/

    return true;
}

bool enqueue(queue_t *self, void *item) {

    /*INVALID PARAMETERS*/
    if(self == NULL || item == NULL){

        errno = EINVAL;
        return false;
    }

    if(self->invalid == true)
        return NULL;


    queue_node_t *node;

    /*MAKE A NEW NODE */
    node = (queue_node_t*)calloc(sizeof(queue_node_t), sizeof(queue_node_t));

    if(pthread_mutex_lock(&self->lock) !=0 ) /*LOCK THE BUFFER*/
        return false;

    /*PUT THE NEWLY CREATED NODE AT THE REAR*/
    node->item = item;
    node->next = self->rear;
    self->rear = node;

    if(pthread_mutex_unlock(&self->lock) != 0) /*UNLOCK THE BUFFER*/
    return false;

    if(sem_post(&self->items) < 0) /*ANNOUNCE AVAILABLE ITEM*/
        return false;

    return true;
}

void *dequeue(queue_t *self) {

     /*INVALID PARAMETERS*/
    if(self == NULL){

        errno = EINVAL;
        return NULL;
    }

    if(self->invalid == true)
        return NULL;

    if(sem_wait(&self->items) < 0) /*WAIT FOR AVAILABLE ITEM*/
        return NULL;

    if(pthread_mutex_lock(&self->lock) != 0) /*LOCK THE BUFFER*/
        return NULL;

    queue_node_t *node = self->front;
    self->front = self->front->next;

    void *item = node->item;
    free(node);

    if(pthread_mutex_unlock(&self->lock) != 0) /*LOCK THE BUFFER*/
        return NULL;

    return item;
}
