#include "utils.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>


#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

int key_exists(hashmap_t *self, map_key_t key);

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {

    hashmap_t *hashmap;

    if(capacity <= 0 || hash_function == NULL || destroy_function == NULL){

        errno = EINVAL;
        return NULL;
    }

    if((hashmap = (hashmap_t*)calloc(1, sizeof(hashmap_t))) == NULL)
        return NULL;

    if(pthread_mutex_init(&hashmap->write_lock, NULL) != 0)
        return NULL;

    if(pthread_mutex_init(&hashmap->fields_lock, NULL) != 0)
        return NULL;

    if((hashmap->nodes = (map_node_t*)calloc(capacity, sizeof(map_node_t))) == NULL)
        return NULL;

    hashmap->capacity = capacity;
    hashmap->size = 0;
    hashmap->hash_function = hash_function;
    hashmap->destroy_function = destroy_function;
    hashmap->num_readers = 0;
    hashmap->invalid = false;

    return hashmap;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {

    bool flag = 0; /*SETTING TO 1 WHEN MAP IS FULL AND FORCE IS FALSE*/
    if(pthread_mutex_lock(&self->write_lock) != 0) /*LOCK THE BUFFER*/
        abort();


    if(self == NULL || self->invalid == true || key.key_base == NULL || key.key_len == 0 || val.val_base == NULL || val.val_len == 0){
        errno = EINVAL;

        if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();

        return false;
    }

    if(self->size == self->capacity && force == false){
        errno = ENOMEM;
        flag = 1;
        goto mapfull;
    }


    int index = get_index(self, key);

    map_node_t *node= ((self->nodes)+index);
    if(self->size < self->capacity){
        if(key_exists(self, key) == -1){ /*Key doesn't exist; find an empty spot to store the key*/
                 while((node->key).key_base != NULL && node->tombstone == false){
                index = (index+1)%self->capacity;
                node = ((self->nodes)+index);
            }
        }

        else{ /*Key does exist; find the key and update the value associated with it*/

            while(memcmp((node->key).key_base, key.key_base, (node->key).key_len) != 0){

                index = (index+1)%self->capacity;
                node = ((self->nodes)+index);
            }

            //self->destroy_function(node->key, node->val); /*USE THE DESTROY_FUNCTION WHEN A NODE IS EVICTED*/
            self->size--;
        }
    }

    if(self->size == self->capacity && force == true){ /*map is full  & force is true; overwrite the value at node supplied by get_index()*/

        //self->destroy_function(node->key, node->val); /*USE THE DESTROY_FUNCTION WHEN A NODE IS EVICTED*/
        self->size--;
    }

    /*SETTING THE VALUE AND KEY N THE APPROPRIATE NODE*/
    node->key = key;
    node->val = val;
    node->tombstone = false;
    self->size = self->size + 1;



    mapfull:

    if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();

    if(flag == 1)
        return false;

    return true;
}

map_val_t get(hashmap_t *self, map_key_t key) {

    if(pthread_mutex_lock(&self->fields_lock) != 0) /*LOCK THE FIELDS*/
        abort();

    self->num_readers++;
    if(self->num_readers == 1){ /*THE FIRST READER LOCKS THE BUFFER*/

        if(pthread_mutex_lock(&self->write_lock) != 0) /*LOCK THE BUFFER*/
            abort();
    }

    if(pthread_mutex_unlock(&self->fields_lock)!= 0) /*UNLOCK THE FIELDS*/
        abort();


        /*ERROR CASE*/
    if(self == NULL || self->invalid == true || key.key_base == NULL || key.key_len == 0){
        errno = EINVAL;

        if(pthread_mutex_lock(&self->fields_lock) != 0) /*LOCK THE FIELDS*/
        abort();

        self->num_readers--;
        if(self->num_readers == 0){

        if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
            abort();
        }

        if(pthread_mutex_unlock(&self->fields_lock)!= 0) /*UNLOCK THE FIELDS*/
            abort();

        return MAP_VAL(NULL, 0);
    }


    int index = get_index(self, key);
    map_val_t value;
    int flag = 0;
    int index_cpy = index; /*THIS IS TO KEEP TRACK IF WE WENT AROUND THE ARRAY TO THE INDEX WE STARTED WITH*/

    map_node_t *node= ((self->nodes)+index);


    if(memcmp((node->key).key_base, key.key_base, (node->key).key_len) == 0) /*key found at the index given by get_index()*/{
        flag = 1 ;
        value = node->val;
    }

    else{

        index++;
        node = ((self->nodes)+index);
        while(index!=index_cpy){

            if(memcmp((node->key).key_base, key.key_base, (node->key).key_len) == 0){
                flag = 1;
                value = node->val;
                break;
            }

            index = (index+1)%self->capacity;
            node = ((self->nodes)+index);
        }
    }

    if(node->tombstone == true) /*IF TOMBSTONE IS SET; THE NODE HAS BEEN DELETED*/
        value = MAP_VAL(NULL, 0);

    if(flag != 1) /*key not found*/
        value = MAP_VAL(NULL, 0);

    if(pthread_mutex_lock(&self->fields_lock) != 0) /*LOCK THE FIELDS*/
        abort();

    self->num_readers--;
    if(self->num_readers == 0){

        if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
            abort();
    }

    if(pthread_mutex_unlock(&self->fields_lock)!= 0) /*UNLOCK THE FIELDS*/
        abort();

    return value;
}

map_node_t delete(hashmap_t *self, map_key_t key) {

    if(pthread_mutex_lock(&self->write_lock) != 0) /*LOCK THE BUFFER*/
        abort();


    /*ERROR CASE*/
    if(self == NULL || self->invalid == true || key.key_base == NULL || key.key_len == 0){

        errno = EINVAL;

        if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();

        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);

    }

    int index = get_index(self, key);
   // int index_cpy = index;

    map_node_t *node_return;


    if((index = key_exists(self, key)) == -1) /*key doesn't exist*/{
        node_return = &MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    else{ /*key exists*/
            map_node_t *node= ((self->nodes)+index);

            if(node->tombstone != true){ /*CHECK IF THIS NODE HAS ALREDAY BEEN DELETED; IF ALREADY DELTED RETURN NULL NODE*/
                node->tombstone = true;
                self->size--;
                node_return = node;
            }

            else
                node_return = &MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);

        }


    if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();

    return *node_return;
}

/*ONLY PUT AND DELETE SHOULD USE THIS FUNCTION BEACUSE THREADS ARE NEVER LOCKED*/
int key_exists(hashmap_t *self, map_key_t key){


    int index = get_index(self, key);
   // map_val_t value;
    int flag = 0;
    int index_cpy = index; /*THIS IS TO KEEP TRACK IF WE WENT AROUND THE ARRAY TO THE INDEX WE STARTED WITH*/

    map_node_t *node= ((self->nodes)+index);

    if(memcmp((node->key).key_base, key.key_base, (node->key).key_len) == 0) /*key found at the index given by get_index()*/{
        flag = 1;
        //value = node->val;
    }

    else{

        index++;
        node = ((self->nodes)+index);
        while(index!=index_cpy){

            if(memcmp((node->key).key_base, key.key_base, (node->key).key_len) == 0){
                flag = 1;
                //value = node->val;
                break;
            }

            index = (index+1)%self->capacity;
            node = ((self->nodes)+index);
        }
    }

    if(flag != 1) /*key not found*/
        return -1;


    return index;
}

bool clear_map(hashmap_t *self) {

    if(pthread_mutex_lock(&self->write_lock) != 0) /*LOCK THE BUFFER*/
        abort();


    if(self == NULL || self->invalid == true){
        errno = EINVAL;
        return false;
        if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();
    }

    map_node_t *node = self->nodes;
    for(int i=0; i< self->capacity; i++){ /*SEARCH THE FULL MAP; A KEY COULD BE PRESENT ANYWHERE*/

        if(((node+i)->key).key_base != NULL && self->size !=0 && ((node+i)->tombstone) != true) /*IF THE KEY EXISTS; IF THE KEY_BASE POINTS TO SOME KEY AND THE MAP HASN'T BEEN CLEARED BEFORE I.E. THE SIZE OF THE MAP IS NOT ZERO AND THE CURRENT NODE HASN'T BEEN DELETED I.E. A TOMBSTONE HAS NOT BEEN SET; BECAUSE THE CREAM SERVER FREES THE KEY_BASE AND VAL_BASE*/{

            (self->destroy_function)((node+i)->key, (node+i)->val);
        }

    }

    self->size = 0; /*RESET SIZE TOO ZERO SINCE THE MAP IS EMPTY*/


    if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();

	return true;
}

bool invalidate_map(hashmap_t *self) {

    if(pthread_mutex_lock(&self->write_lock) != 0) /*LOCK THE BUFFER*/
        abort();

    if(pthread_mutex_lock(&self->fields_lock) != 0) /*LOCK THE FIELDS*/
        abort();

    if(self == NULL || self->invalid == true){

        errno = EINVAL;
        return false;
    }

    map_node_t *node = self->nodes;

    for(int i=0; i< self->size; i++){

        (self->destroy_function)((node+i)->key, (node+i)->val);
    }

    self->size = 0; /*RESET SIZE TOO ZERO SINCE THE MAP IS EMPTY*/

    free(self->nodes); /*FREE THE NODES POINTER*/

    self->invalid = true;

    if(pthread_mutex_unlock(&self->fields_lock) != 0) /*UNLOCK THE FIELDS*/
        abort();

    if(pthread_mutex_unlock(&self->write_lock) != 0) /*UNLOCK THE BUFFER*/
        abort();

    return true;
}
