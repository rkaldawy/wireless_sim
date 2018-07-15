#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <ncurses.h>
#include "proj3hdr.h"

//a cool stub to see how a single network runs in our system
void test_add_nodes(){
  //add_node(ONE, 16, 15, LEGITIMATE);
  add_node(ONE, 15, 13, LEGITIMATE);
  add_node(ONE, 13, 13, LEGITIMATE);
  add_node(ONE, 9, 9, LEGITIMATE);
  add_node(ONE, 5, 3, LEGITIMATE);
  add_node(ONE, 7, 5, LEGITIMATE);
  add_node(ONE, 5, 5, LEGITIMATE);
  add_node(ONE, 3, 5, LEGITIMATE);
  add_node(ONE, 12, 5, LEGITIMATE);
  add_node(ONE, 13, 4, LEGITIMATE);

  /*add_node(ONE, 4, 10, LEGITIMATE);
  add_node(THREE, 13, 13, LEGITIMATE);
  add_node(THREE, 15, 11, LEGITIMATE);
  add_node(ELEVEN, 15, 8, LEGITIMATE);
  add_node(ELEVEN, 11, 5, LEGITIMATE);
  add_node(ELEVEN, 10, 11, NOISEMAKER);
  */
}

void print_nodes_in_queue(NodeQueue* queue){
  int i = 1;
  while(queue != NULL && queue->next != NULL){
    printf("The %d node in our waiting queue is: (%d, %d)\n", i, queue->next->node->x_pos, queue->next->node->y_pos);
    queue = queue->next;
    i++;
  }
}

void test_node_queue(){
  add_node(ONE, 12, 10, LEGITIMATE);
  add_node(ONE, 10, 30, LEGITIMATE);
  add_node(ONE, 8, 50, LEGITIMATE);
  add_node(ONE, 40, 40, LEGITIMATE);

  gettimeofday(&field[12][10]->start_time, NULL);

  unsigned int retTime = time(0) + 1;   // Get finishing time.
  while (time(0) < retTime);

  gettimeofday(&field[8][50]->start_time, NULL);

  retTime = time(0) + 1;   // Get finishing time.
  while (time(0) < retTime);

  gettimeofday(&field[40][40]->start_time, NULL);

  retTime = time(0) + 1;   // Get finishing time.
  while (time(0) < retTime);

  gettimeofday(&field[10][30]->start_time, NULL);



  printf("Testing the addition and removal fn's.\n");
  add_by_time(field[10][30]->waiting_nodes, field[8][50]);
  add_by_time(field[10][30]->waiting_nodes, field[12][10]);
  add_by_time(field[10][30]->waiting_nodes, field[40][40]);

  print_nodes_in_queue(field[10][30]->waiting_nodes);

  printf("We added! Now let's remove.\n");
  clear_first_node_queue(field[10][30]->waiting_nodes);
  print_nodes_in_queue(field[10][30]->waiting_nodes);
  printf("Removing second...\n");
  clear_first_node_queue(field[10][30]->waiting_nodes);
  print_nodes_in_queue(field[10][30]->waiting_nodes);
  printf("Removing third...\n");
  clear_first_node_queue(field[10][30]->waiting_nodes);
  print_nodes_in_queue(field[10][30]->waiting_nodes);
  printf("And.. we removed all nodes! great!\n");
}

//to be used when n nodes and noisemakers are desired randmly
void test_add_rand_nodes(){
  add_rand_nodes();
  //initialize_threads();
}

void test_fill_nodes(){
  for(int i = 1; i < 13; i++){
    for(int j = 1; j < 13; j++){
      add_node(ONE, i, j, LEGITIMATE);
    }
  }
}
