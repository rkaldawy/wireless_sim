#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include "proj3hdr.h"

void initialize_nodes(){
  initialize_field();
}

//this function starts a thread for each node
void initialize_threads(){
  for (int i = 0; i < 100; i++){
    for (int j = 0; j < 100; j++){
      if (field[i][j] != NULL){
        Node* me = field[i][j]; //set up a node in the position of the array
        printf("Hey now, we found a node at (%d, %d)!\n", me->x_pos, me->y_pos);
        //pthread_t p;
        pthread_create(&me->thread, NULL, node_thread, me); //start up its thread
        //pthread_create(&p, NULL, test_thread, NULL);
      }
    }
  }
}

//whenever a new node is added, this function gives it pointers to all its nodes
void find_neighbors(Node* node_ptr, int x_pos, int y_pos){

  int minX = 5;
  int maxX = 5;
  int minY = 5;
  int maxY = 5;
  if (x_pos < 5){ minX = x_pos; }
  else if (x_pos > 94) { maxX = 99 - x_pos; }
  if (y_pos < 5){ minY = y_pos; }
  else if (y_pos > 94) { maxY = 99 - y_pos; }

  Node** source_array; Node** target_array;
  int* source_array_size; int* target_array_size;

  for (int i = (x_pos-minX); i <= (x_pos+maxX); i++){
    for (int j = (y_pos-minY); j <= (y_pos+maxY); j++){
      //find and link the two neighbors
      if (field[i][j] != NULL){
        if (field[i][j]->type == LEGITIMATE){
          source_array = node_ptr->nbrs;
          source_array_size = &node_ptr->nbr_size;
        } else if (field[i][j]->type == NOISEMAKER){
          source_array = node_ptr->nsmkr_nbrs;
          source_array_size = &node_ptr->nsmkr_nbr_size;
        }
        if (node_ptr->type == LEGITIMATE){
          target_array = field[i][j]->nbrs;
          target_array_size = &field[i][j]->nbr_size;
        } else if (node_ptr->type == NOISEMAKER){
          source_array = field[i][j]->nsmkr_nbrs;
          target_array_size = &field[i][j]->nsmkr_nbr_size;
        }

        printf("We found that the node at (%d, %d) has a neighbor at (%d, %d).\n", x_pos, y_pos, i, j);
        source_array[*source_array_size] = field[i][j];
        target_array[*target_array_size] = node_ptr;
        (*source_array_size)++;
        (*target_array_size)++;

        /*        node_ptr->nbrs[node_ptr->nbr_size] = field[i][j];
        field[i][j]->nbrs[field[i][j]->nbr_size] = node_ptr;
        node_ptr->nbr_size++;
        field[i][j]->nbr_size++;*/
      }
    }
  }
}

//this initializes all the data for a node
void add_node(int frequency, int x_pos, int y_pos, int type){
  Node* node_ptr = (Node*) malloc(sizeof(Node)); //reserve it memory
  node_ptr->channel = frequency;
  node_ptr->x_pos = x_pos;
  node_ptr->y_pos = y_pos;
  node_ptr->type = type;
  node_ptr->shield = 0; //sheilds always start a 0
  node_ptr->action = NEITHER;
  node_ptr->interference = CLEAR; //start with nobody bothering us
  node_ptr->nbr_size = 0;
  node_ptr->nsmkr_nbr_size = 0;
  node_ptr->waiting_nodes = initialize_node_queue();
  node_ptr->color = (rand()%15) + 1;
  /*
  node_ptr->id = CURRENT_NUM; //each node has a unique "id"
  CURRENT_NUM++;

  node_ptr->MSGS_COUNT = 0;
  node_ptr->message_list = (MessageList*)malloc(sizeof(MessageList)); //we malloc our list of messages
  initialize_message_list(node_ptr->message_list); //we set up the head and tail pointers within
  node_ptr->file = NULL;
  */

  //reassign the attributes of our locks

  pthread_mutex_init(&node_ptr->shield_lock, NULL);
  //pthread_mutexattr_init(&node_ptr->action_attr);
  //pthread_mutexattr_settype(&node_ptr->action_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&node_ptr->action_lock, NULL);
  //pthread_mutexattr_init(&node_ptr->noise_attr);
  //pthread_mutexattr_settype(&node_ptr->noise_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&node_ptr->noise_lock, NULL);
  //pthread_mutexattr_init(&node_ptr->msgcount_attr);
  //pthread_mutexattr_settype(&node_ptr->msgcount_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&node_ptr->msgcount_lock, NULL);

  //initialize the condition bariable
  pthread_cond_init(&node_ptr->cond, NULL);

  //look for our neighbors and put the node in the array
  find_neighbors(node_ptr, x_pos, y_pos);

  field[x_pos][y_pos] = node_ptr;
}

NodeQueue* initialize_node_queue(){
  NodeQueue* new_queue = (NodeQueue*)malloc(sizeof(NodeQueue));
  new_queue->node = NULL;
  pthread_cond_init(&new_queue->cond, NULL);
  new_queue->prev = NULL;
  new_queue->next = NULL;
  return new_queue;
}

//adds a new node by the its incident time to the array.
pthread_cond_t* add_by_time(NodeQueue* queue, Node* new_node){

  //NodeQueue* queue = queue_orig;
  struct timeval current_time;
  gettimeofday(&current_time, NULL);

  int incoming_elapsed = ((current_time.tv_sec * 1000 + current_time.tv_usec / 1000) - (new_node->start_time.tv_sec * 1000 + new_node->start_time.tv_usec / 1000));
  //printf("the incoming elapsed time is: %d\n", incoming_elapsed);
  int queue_node_elapsed;

  NodeQueue* new_elt = initialize_node_queue();
  new_elt->node = new_node;
  pthread_cond_init(&new_elt->cond, NULL);
  if (queue->next == NULL){
    new_elt->prev = queue;
    queue->next = new_elt;
    //printf("%p\n", (void*)&new_elt->cond);
    return &new_elt->cond;
  }
  queue = queue->next;

  while(1){
    queue_node_elapsed = ((current_time.tv_sec * 1000 + current_time.tv_usec / 1000) - (queue->node->start_time.tv_sec * 1000 + queue->node->start_time.tv_usec / 1000));
    //printf("the current node's elapsed time is: %d\n", queue_node_elapsed);
    if (incoming_elapsed >= queue_node_elapsed){
      //printf("We made it.\n");
      new_elt->prev = queue->prev;
      new_elt->next = queue;
      queue->prev->next = new_elt;
      queue->prev = new_elt;
      return &new_elt->cond;
    }
    if (queue->next == NULL){ break; }
    else { queue = queue->next; }
  }

  queue->next = new_elt;
  new_elt->prev = queue;
  new_elt->next = NULL;
  return &new_elt->cond;
}

//clear the first node from the queue of nodes waiting to make a network
int clear_first_node_queue(NodeQueue* queue){

  if (queue->next == NULL){ return 0; }
  else if (queue->next->next == 0) {
    pthread_cond_signal(&queue->next->cond);
    queue->next = NULL;
    free(queue->next);
    return 1;
  }
  else {
    NodeQueue* first = queue->next;
    pthread_cond_signal(&first->cond);
    first->next->prev = queue;
    queue->next = first->next;
    free(first);
    return 1;
  }
}

VisitCheck* initialize_visit_list(Node* node_ptr){
  VisitCheck* visit_list = (VisitCheck*) calloc(node_ptr->nbr_size, sizeof(VisitCheck));
  for(int i = 0; i < node_ptr->nbr_size; i++){
    visit_list[i].visited = 0;
    pthread_cond_init(&visit_list[i].cond, NULL);
  }
  return visit_list;
}

int find_position_in_array(Node* to_be_found, Node* search_in){
  int pos = -1;
  for (int i = 0; i < search_in->nbr_size; i++){
    if (to_be_found->x_pos == search_in->nbrs[i]->x_pos && to_be_found->y_pos == search_in->nbrs[i]->y_pos){
      pos = i;
    }
  }
  return pos;
}

int check_visited_full(Node* node_ptr){
  for(int i = 0; i < node_ptr->nbr_size; i++){
    //printf("%d, %d\n", node_ptr->nbrs[i]->x_pos, node_ptr->nbrs[i]->y_pos);
    //printf("%d\n", node_ptr->visit_list[i].visited);
    if (node_ptr->visit_list[i].visited == 0){
      return 0;
    }
  }
  return 1;
}

void wake_all_visited(Node* node_ptr){
  pthread_mutex_lock(&node_ptr->shield_lock);
  for(int i = 0; i < node_ptr->nbr_size; i++){
    node_ptr->visit_list[i].visited  = 0;
  }
  for(int i = 0; i < node_ptr->nbr_size; i++){
    pthread_cond_signal(&node_ptr->visit_list[i].cond);
  }
  pthread_mutex_unlock(&node_ptr->shield_lock);
}


//build the field where the nodes will be created
void initialize_field(){
  //the field starts with nulls
  for (int i = 0; i < 100; i++){
    for (int j = 0; j < 100; j++){
      field[i][j] = NULL;
    }
  }
}
