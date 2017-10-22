#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <ncurses.h>
#include "proj3hdr.h"

void initialize_values(){
  //set the randomizer
  srand(time(NULL));
  //please set your values here
  num_nodes = 500;
  num_noisemakers = 50;
  dwell_duration = 100;
  dwell_probability = 0;
  transmission_time = 1000;
  talk_window_time = 100;
  talk_probability = 25;
  dwell_noisemakers = 100;
  dwell_probability_noisemakers = 10;
  block_probability = 75;

  pthread_mutex_init(&draw_lock, NULL);
  //the messages we could send
}

///NEEDS CHANGING
void initialize_display(){

  initscr();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  noecho();
  start_color();

  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_CYAN, COLOR_BLACK);
  init_pair(6, COLOR_CYAN, COLOR_BLACK);
  init_pair(7, COLOR_WHITE, COLOR_BLACK);

  for (int i = 0; i < 200; i += 2){
    for (int j = 0; j < 100; j+= 1){
      mvaddch(j, i, 'O');
    }
  }
  refresh();
}

int main(){
  initialize_values();
  //test_fill_nodes();
  //test_add_nodes();
  test_add_rand_nodes();
  initialize_structs();
  initialize_display();
  initialize_threads();

  while(1){};
}
