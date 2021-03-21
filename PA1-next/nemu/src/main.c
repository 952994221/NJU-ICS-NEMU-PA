#include "monitor/expr.h" // include expr.h to test expression evaluation program
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);
  /* TODO:test your expression evaluation program in here
   * add your test code to test all expression
   * expression are in the file at nemu/tools/gen-expr/input.txt
   */

  puts("begin to test expr.c\n");
  bool success = true;
  FILE * fp = NULL;
  char buf[256]={0};
  char content[256] = {0};
  uint32_t ans = 0;
  fp = fopen("/home/misaki/ics2020/nemu/src/psmd.txt", "r");
  if(fp == NULL) {puts("No such file as psmd.txt\n");return 1;}
  while(!feof(fp)){
    memset(buf, 0, sizeof(buf));
    char * read = fgets(buf, sizeof(buf)-1, fp); // read a line
    printf("read info: %s\n", read);
    sscanf(buf, "%u%[^\n]", &ans, content);
    uint32_t res = expr(content, &success);
    printf("testing expr: %s, answer: %u, your result: %u\n\n", content, ans, res);
    printf("--------------------------------------------------------------\n\n");
    if(res != ans){
      printf("error when testing expr: %s, answer is %u, your result is %u\n", content, ans, res);
      return 1;
    }
  }
  puts("Expression evaluation testing finished!\n");
  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
