#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  bool is_free;
  int NO;
  char msg[100];
  int val; // value of watch
  struct watchpoint *next;

  /* TODO: Add more members if necessary */


} WP;

/* TODO: if necessary, try to implement these functions freely in watchpoint.c
 * you can add other functions by yourself if necessary
 */

 WP * new_wp(char *, int );
 void free_wp(int);
 void print_wp();
 void delete_all_wp();
 int check_wp();
#endif
