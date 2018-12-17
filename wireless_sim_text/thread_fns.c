#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "proj3hdr.h"

//puts a shield on all our neighbors so that we can test them
//return 1 on success and 0 on failure

/*for(int j = i-1; j >= 0; j--){
  past_neighbor = me->nbrs[j];
  pthread_mutex_lock(&past_neighbor->shield_lock);
  past_neighbor->shield = 0;
  wake_from_node(past_neighbor);
  pthread_mutex_unlock(&past_neighbor->shield_lock);

}*/

/*
  NodeQueue* queue_copy = queue;

  while(1){

    if (queue_copy->next == NULL){
      break;
    }
    printf("(%d, %d)\n", queue_copy->next->node->x_pos, queue_copy->next->node->y_pos);
    queue_copy = queue_copy->next;
  }

  */

int wait_to_node(Node* queuer, Node* queued, pthread_mutex_t* lock){
  NodeQueue* queue = queuer->waiting_nodes;
  pthread_cond_t* cond = add_by_time(queue, queued);
  //printf("%p\n", (void*)cond);
  pthread_cond_wait(cond, lock);
  printf("The node at (%d, %d) is awoken!\n", queued->x_pos, queued->y_pos);
}

int wake_from_node(Node* queuer){
  printf("We are waking all elements the node (%d, %d)!\n", queuer->x_pos, queuer->y_pos);
  NodeQueue* queue = queuer->waiting_nodes;

  int empty;
  do{
    empty = clear_first_node_queue(queue);
    //printf("The queue at (%d, %d) has %d elements letf\n", queuer->x_pos, queuer->y_pos, empty);
  }
  while (empty);
}


int build_shields(Node* me){
  Node* neighbor;

  printf("The node at (%d, %d) is trying to set up its shields!\n", me->x_pos, me->y_pos);
  printf(" The node at (%d, %d) has the current shield value: %d \n", me->x_pos, me->y_pos, me->shield);
  for (int i = 0; i < me->nbr_size; i++){
    printf("    The neighbor at (%d, %d) has this current shield value: %d and the current channel value: %d\n", me->nbrs[i]->x_pos,
                                                                                                                 me->nbrs[i]->y_pos,
                                                                                                                 me->nbrs[i]->shield,
                                                                                                                 me->nbrs[i]->channel);
  }
  //Put the shield on myself, so no other node can access me while I'm putting shields on my neighbors
  pthread_mutex_lock(&me->shield_lock);
  if (me->shield == 1) {
    printf("Someone else is already locking me! I am (%d, %d).\n", me->x_pos, me->y_pos);
    wait_to_node(me, me, &me->shield_lock);
    //unsigned int retTime = time(0) + 1;   // Get finishing time.
    //while (time(0) < retTime);
    pthread_mutex_unlock(&me->shield_lock);
    return 0;
  }
  me->shield = 1;
  pthread_mutex_unlock(&me->shield_lock);

  printf("The node at (%d, %d) has set up its own shield!\n", me->x_pos, me->y_pos);

  //try to put up our shields on every neighbor
  for (int i = 0; i < me->nbr_size; i++){
    neighbor = me->nbrs[i];

    printf("BING\n");
    //THE PROBLEM IS THIS LOCK
    pthread_mutex_lock(&neighbor->shield_lock);
    printf("BONG\n");

    //if someone else has aleady shielded this node:
    if (neighbor->visit_list[find_position_in_array(me, neighbor)].visited == 1){
      printf("I am (%d, %d), and I need to give someone else a turn first. \n", me->x_pos, me->y_pos);

      Node* past_neighbor;

      for(int j = i-1; j >= 0; j--){
        past_neighbor = me->nbrs[j];
        printf("testestest\n");
        pthread_mutex_lock(&past_neighbor->shield_lock);
        past_neighbor->shield = 0;
        printf("We are waking a past neighbor\n");
        wake_from_node(past_neighbor);
        pthread_mutex_unlock(&past_neighbor->shield_lock);
      }
      printf("hoigois\n");

      pthread_mutex_lock(&me->shield_lock);
      me->shield = 0;
      wake_from_node(me);
      pthread_mutex_unlock(&me->shield_lock);

      pthread_cond_wait(&neighbor->visit_list[find_position_in_array(me, neighbor)].cond, &neighbor->shield_lock);

      pthread_mutex_unlock(&neighbor->shield_lock);

      //do we need to add a return here? CHECK THIS LATER
      return 0;
    }
    else if (neighbor->shield == 1) {
      printf("I am (%d, %d), and there's already a shield at (%d, %d).\n", me->x_pos, me->y_pos, neighbor->x_pos, neighbor->y_pos);
      //put down all shields I have put up so far
      Node* past_neighbor;

      for(int j = i-1; j >= 0; j--){
        past_neighbor = me->nbrs[j];
        pthread_mutex_lock(&past_neighbor->shield_lock);
        past_neighbor->shield = 0;
        printf("We are waking a past neighbor\n");
        wake_from_node(past_neighbor);
        pthread_mutex_unlock(&past_neighbor->shield_lock);
      }

      pthread_mutex_unlock(&neighbor->shield_lock);
      pthread_mutex_lock(&me->shield_lock);
      printf("I am (%d, %d), and I am letting go of my shield.\n", me->x_pos, me->y_pos);
      me->shield = 0;
      wake_from_node(me);
      pthread_mutex_unlock(&me->shield_lock);

      wait_to_node(neighbor, me, &neighbor->shield_lock);
      //THIS LINE IS ESSENTIAL
      pthread_mutex_unlock(&neighbor->shield_lock);
      return 0;
    }
    else {
        printf("We are adding a shield to (%d, %d).\n", neighbor->x_pos, neighbor->y_pos);
        neighbor->shield = 1;
      }
    pthread_mutex_unlock(&neighbor->shield_lock);
    }

  return 1;
}

void release_nodes(Node* me){
  Node* neighbor;
  //we can let go of our shields
  for (int k = 0; k < me->nbr_size; k++){
    neighbor = me->nbrs[k];
    int my_index = find_position_in_array(me, neighbor);
    neighbor->visit_list[my_index].visited = 1;

    if (check_visited_full(neighbor)){
      wake_all_visited(neighbor);
    }
  }

  for (int j = 0; j < me->nbr_size; j++){
    neighbor = me->nbrs[j];
    pthread_mutex_lock(&neighbor->shield_lock);
    neighbor->shield = 0;
    wake_from_node(neighbor);//need to finda  way to track who is locking a node
    pthread_mutex_unlock(&neighbor->shield_lock);

  }

  printf("The node at (%d, %d) has released all its neighbor's nodes!\n", me->x_pos, me->y_pos);
  // undo yourself finally
  pthread_mutex_lock(&me->shield_lock);
  me->shield = 0;
  wake_from_node(me);
  pthread_mutex_unlock(&me->shield_lock);
}

void* node_thread(void* node_void_ptr){

  Node* me = (Node*) node_void_ptr;
  struct timeval start, end, msg_start, msg_end, think_start, think_end;
  time_t now;

  printf("We are starting! This node is at (%d, %d)\n", me->x_pos, me->y_pos);

  while(1){

    gettimeofday(&me->start_time, NULL);
    printf("\nThe node at (%d, %d) has a channel of: %d!\n", me->x_pos, me->y_pos, me->channel);
    //pthread_mutex_lock(&me->channel_lock);
    int shields_built = 0;
    int nodes_free = 0;
    //keep looping until we have all our shields and none of the neighbors are in a network anymore
    //while(!nodes_free){
    while (!shields_built){
      shields_built = build_shields(me);
    }
    printf("The node at (%d, %d) has set up its neighbors' shields!\n", me->x_pos, me->y_pos);

    //for (int i = 0; i < me->nbr_size; i++){
    //  printf("    The neighbor at (%d, %d) has this current shield value: %d and the current channel value: %d\n", me->nbrs[i]->x_pos, me->nbrs[i]->y_pos, me->nbrs[i]->shield,  me->nbrs[i]->channel);
    //}


    printf("\n");
    printf("The node at (%d, %d) has begun transmission!\n", me->x_pos, me->y_pos);
    printf("\n");

    unsigned int retTime = time(0) + 1;   // Imagine a message is being sent.
    while (time(0) < retTime);

    printf("\n");
    printf("The node at (%d, %d) has ended transmission!\n", me->x_pos, me->y_pos);
    printf("\n");

    release_nodes(me);
  }

  return NULL;
}




////////
////////
////////
/*
void noisemaker_thread(Node* me){

  struct timeval start, end, dwell_start, dwell_end;
  Node* neighbor;


  while (1){
    //see if we need to change the channel of the noise maker
    check_channel_change_noisemaker(&dwell_start, &dwell_end, me);
    //increment the number of noisemakers talking to each node
    for(int i = 0; i < me->nbr_size; i++){
      neighbor = me->nbrs[i];
      if (me->channel == neighbor->channel){
        pthread_mutex_lock(&neighbor->noise_lock);
        neighbor->interference++;
        pthread_mutex_unlock(&neighbor->noise_lock);
      }
    }
    //see how long we will block our neighbors
    noisemaker_wait(&start, &end);
    //decrement the number of noisemakers in our neighbors because we are done blocking.
    for(int i = 0; i < me->nbr_size; i++){
      if (me->channel == neighbor->channel){
        //decrement the interference count
        pthread_mutex_lock(&neighbor->noise_lock);
        neighbor->interference--;
        pthread_cond_broadcast(&neighbor->cond);
        pthread_mutex_unlock(&neighbor->noise_lock);
      }
    }
  }

}
*/
