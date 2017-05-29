#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "proj3hdr.h"

//puts a shield on all our neighbors so that we can test them
//return 1 on success and 0 on failure

int wait_to_node(Node* queuer, Node* queued, pthread_mutex_t lock){
  NodeQueue* queue = queuer->waiting_nodes;
  pthread_cond_t cond = add_by_time(queue, queued);
  pthread_cond_wait(&cond, &lock);
  printf("We are locking!\n");
}

int wake_from_node(Node* queuer){
  NodeQueue* queue = queuer->waiting_nodes;
  int empty;
  do{
    empty = clear_first_node_queue(queue);
  }
  while (empty);
}


int build_shields(Node* me){
  Node* neighbor;
  printf("The node at (%d, %d) is trying to set up its shields!\n", me->x_pos, me->y_pos);

  printf(" The node at (%d, %d) has the current shield value: %d \n", me->x_pos, me->y_pos, me->shield);
  for (int i = 0; i < me->nbr_size; i++){
    printf("    The neighbor at (%d, %d) has this current shield value: %d and the current channel value: %d\n", me->nbrs[i]->x_pos, me->nbrs[i]->y_pos, me->nbrs[i]->shield,  me->nbrs[i]->channel);
  }
  //Put the shield on myself, so no other node can access me while I'm putting shields on my neighbors
  pthread_mutex_lock(&me->shield_lock);
  if (me->shield == 1) {
    wait_to_node(me, me, me->shield_lock);

    printf("We have locked!\n");
    //pthread_cond_wait(&me->cond, &me->shield_lock);
  }
  pthread_mutex_unlock(&me->shield_lock);

  me->shield = 1;

  printf("The node at (%d, %d) has set up its own shield!\n", me->x_pos, me->y_pos);

  //try to put up our shields on every neighbor
  for (int i = 0; i < me->nbr_size; i++){
    neighbor = me->nbrs[i];
    // Check whether a neighbor has the same channel frequency
    pthread_mutex_lock(&neighbor->shield_lock);
    printf("testestest\n");
    //if someone else has aleady shielded this node:
    if (neighbor->shield == 1 && neighbor->channel == me->channel) {
      //put down all shields I have put up so far
      Node* past_neighbor;

      /*for (int j = (i-1); j >= 0; j--){
        past_neighbor = me->nbrs[j];
        if (past_neighbor->channel == me->channel){
          pthread_mutex_lock(&past_neighbor->shield_lock);
          past_neighbor->shield = 0;
          wake_from_node(past_neighbor);
          pthread_mutex_unlock(&past_neighbor->shield_lock);
        }
      }*/
      //finally release my own shield
      //pthread_mutex_lock(&me->shield_lock);
      me->shield = 0;
      wake_from_node(me);
      //pthread_mutex_unlock(&me->shield_lock);

      //Put myself to sleep and unlock myself, wait until the node we are waiting for releases its shield
      wait_to_node(neighbor, me, neighbor->shield_lock);
      pthread_mutex_unlock(&neighbor->shield_lock);
      return 0;
    }
    else {
      //if a shield isnt already there, now we get to put a shield on it, and the neighbor is reserved for us
      if (neighbor->channel == me->channel){
        neighbor->shield = 1;
      }
      pthread_mutex_unlock(&neighbor->shield_lock);
    }
  }
  return 1;
}

//check that all the nodes are currently not transmitting

//REMY: PROBLEM IS IN THIS FUNCTION>>> SOMEWHERE
/*int read_nodes(Node* me){
  Node* neighbor;
  //we first set up a lock on each neighbor so that it doesnt change its action while we check it
  for (int i = 0; i < me->nbr_size; i++){
    neighbor = me->nbrs[i];
    pthread_mutex_lock(&neighbor->action_lock);

    //if a neighbor is currently in a network, put down all our shields and wait until it has stopped
    if (neighbor->action != NEITHER && neighbor->channel == me->channel){
      Node* past_neighbor;
      //let go of all our relevant shields
      for (int j = 0; j < me->nbr_size; j++){
        past_neighbor = me->nbrs[j];
        //make sure we lock the node before we begin
        if (past_neighbor->channel == me->channel){
          pthread_mutex_lock(&past_neighbor->shield_lock);
          past_neighbor->shield = 0;
          pthread_mutex_unlock(&past_neighbor->shield_lock);

          wake_from_node(past_neighbor);
          //pthread_cond_broadcast(&past_neighbor->cond);
        }
      }
      //release our own shield
      pthread_mutex_lock(&me->shield_lock);
      me->shield = 0;
      wake_from_node(me);
      //pthread_cond_broadcast(&me->cond);
      pthread_mutex_unlock(&me->shield_lock);
      //wait until the neighbor stops being in the network
      while (neighbor->action != NEITHER){
        pthread_cond_wait(&neighbor->cond, &neighbor->action_lock);
      }
      pthread_mutex_unlock(&neighbor->action_lock);
      return 0;
    }
    else {
      pthread_mutex_unlock(&neighbor->action_lock);
    }
  }
  return 1;
}
*/

void* node_thread(void* node_void_ptr){

  Node* me = (Node*) node_void_ptr;
  struct timeval start, end, msg_start, msg_end, think_start, think_end;
  time_t now;

  // a whole separate protocol if we are a noisemaker
  //if (me->type == NOISEMAKER){
  //  noisemaker_thread(me);
  //}

  gettimeofday(&me->start_time, NULL);

  printf("We are starting! This node is at (%d, %d)\n", me->x_pos, me->y_pos);

  while(1){
    //run the transmission_wait code to see if we will send a message this time
    /*
    int can_talk = try_message(&think_start, &think_end);
    if (can_talk){ //if not, just try again
      continue;
    }
    */

    //add a new message to transmit
    //now = time(0);
    //add_transmit(me->message_list, me->file, me->id, now, me->x_pos, me->y_pos);
    //see if it is time to change channels
    //check_channel_change(&start, &end, me);
    printf("\nThe node at (%d, %d) has a channel of: %d!\n", me->x_pos, me->y_pos, me->channel);
    //pthread_mutex_lock(&me->channel_lock);
    int shields_built = 0;
    int nodes_free = 0;
    //keep looping until we have all our shields and none of the neighbors are in a network anymore
    //while(!nodes_free){
      while (!shields_built){
        shields_built = build_shields(me);
        printf("The node at (%d, %d) has set up its neighbors' shields!\n", me->x_pos, me->y_pos);
      }

      for (int i = 0; i < me->nbr_size; i++){
        printf("    The neighbor at (%d, %d) has this current shield value: %d and the current channel value: %d\n", me->nbrs[i]->x_pos, me->nbrs[i]->y_pos, me->nbrs[i]->shield,  me->nbrs[i]->channel);
      }

      //at this point, all locks are released and we have all shields in place
      /*nodes_free = read_nodes(me);
      printf("Sire, we have failed!\n");
    }*/
    printf("The node at (%d, %d) has checked all its nodes!\n", me->x_pos, me->y_pos);

    unsigned int retTime = time(0) + 2;   // Get finishing time.
    while (time(0) < retTime);

    Node* neighbor;
    //now all our neighbors are recieving messages
    /*for (int i = 0; i < me->nbr_size; i++){
      neighbor = me->nbrs[i];
      if (neighbor->channel == me->channel){
        pthread_mutex_lock(&neighbor->action_lock);
        neighbor->action = RECIEVING;
        pthread_mutex_unlock(&neighbor->action_lock);
      }
    }
    printf("The node at (%d, %d) has set all its neighbor's nodes!\n", me->x_pos, me->y_pos);
*/
    //we can let go of our shields
    for (int j = 0; j < me->nbr_size; j++){
      neighbor = me->nbrs[j];
      if (neighbor->channel == me->channel){
        //pthread_mutex_lock(&neighbor->shield_lock);
        me->nbrs[j]->shield = 0;
        printf("We are waking.\n");
        wake_from_node(neighbor);
        printf("We have stopped waking.\n");

      }
    }

    printf("The node at (%d, %d) has released all its neighbor's nodes!\n", me->x_pos, me->y_pos);
    // undo yourself finally
    //pthread_mutex_lock(&me->shield_lock);
    me->shield = 0;
    wake_from_node(me);
    //pthread_cond_broadcast(&me->cond);
    //pthread_mutex_unlock(&me->shield_lock);
/*
    printf("The node at (%d, %d) is sending a message!\n", me->x_pos, me->y_pos);

    //// lets see if we will send a message, and what that message will be
    //transmission_wait(&msg_start, &msg_end, me);

    ////
    //finally, we stop transmiting to our relevant neighbors
    for (int i = 0; i < me->nbr_size; i++){
      neighbor = me->nbrs[i];
      if (neighbor->channel == me->channel){
        pthread_mutex_lock(&neighbor->action_lock);
        neighbor->action = NEITHER;
        pthread_cond_broadcast(&neighbor->cond);
        pthread_mutex_unlock(&neighbor->action_lock);
      }
    }
    printf("The node at (%d, %d) has disconnected its network!\n", me->x_pos, me->y_pos);
    //pthread_mutex_unlock(&me->channel_lock);
    */
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
