#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};

// TODO: try to re-organize, you can abort head and free_ pointer while just use static int index
static WP *head, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i+1;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].is_free = true;
  }
  wp_pool[NR_WP - 1].next = NULL;
  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char * msg, int val){
  if(free_ == NULL)
  {
      printf("\033[1m\033[31mWatchpoint array is full\033[0m\n");
      return NULL;
  }
  WP* p = free_;
  free_ = free_->next;
  strcpy(p->msg, msg);
  p->val = val;
  p->is_free = false;
  p->next = NULL;
  WP* h = head;
  if(h == NULL)
  {
    head = p;
    h = head;
  }
  else
  {
    while(h->next != NULL)
      h = h->next;
    h->next = p;
  }
  printf("\033[1m\033[33mWatchpoint %d\texpr:\033[0m %s \033[1m\033[33mval:\033[0m 0x%08x\n", p->NO, p->msg, p->val);
  return p;
}

void free_wp(int no){
  if(no > NR_WP || no < 1)
  {
    printf("\033[1m\033[31mInvalid breakpoint\033[0m\n");
    return;
  }
  if(wp_pool[no-1].is_free == true)
  {
    printf("\033[1m\033[31mNo such watchpoint, numbered as %d\033[0m\n", no);
    return;
  }
  WP* p = head;
  if(p->NO == no)
    head = head->next;
  else
  {
    //while(p->next != NULL && p->next->NO != no)
    while(p->next->NO != no)
      p = p->next;
    p->next = p->next->next;
  }
  wp_pool[no - 1].next = free_;
  wp_pool[no - 1].is_free = true;
  wp_pool[no - 1].msg[0]= '\0';
  wp_pool[no - 1].val = 0;
  free_ = &wp_pool[no - 1];
  printf("\033[1m\033[32mDelete watchpoint %d\033[0m\n", no);
  return;
}

void print_wp(){
  WP* p = head;
  if(p == NULL)
    printf("\033[1m\033[31mNo watchpoints\033[0m\n");
  else
  {
    while(p != NULL)
    {
      printf("\033[1m\033[33mWatchpoint %d\texpr:\033[0m %s \033[1m\033[33mval:\033[0m 0x%08x\n", p->NO, p->msg, p->val);
      p = p->next;
    }
  }
  return;
}

void delete_all_wp(){
  int i;
  for(i = 1; i <= NR_WP; i++)
  {
    if(wp_pool[i - 1].is_free == false)
      free_wp(i);
  }
  return;
}

int check_wp()
{
  WP* p = head;
  bool success;
  int hit = false;
  while(p != NULL)
  {
    int new_val = expr(p->msg, &success);
    if(success == false)
      printf("\033[1m\033[31mWatchpoint %d is invalid\033[0m\n", p->NO);
    if(new_val != p->val)
    {
      printf("\033[1m\033[33mWatchpoint %d:\033[0m %s\n", p->NO, p->msg);
      printf("\033[1m\033[33mOld value =\033[0m 0x%08x\n\033[1m\033[33mNew value =\033[0m 0x%08x\n", p->val, new_val);
      p->val = new_val;
      hit = true;
    }
    p = p->next;
  }
  return hit;
}
