#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536]="";
static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
// TODO: implement these functions: choose, gen_rand_op, gen_num, gen_rand_expr
static inline uint32_t choose(uint32_t n) {
  return rand() % n;
}

static inline void gen_rand_op(){
  switch(choose(8))
  {
    case 0:
      sprintf(buf + strlen(buf), "%s", "+");
      break;
    case 1:
      sprintf(buf + strlen(buf), "%s", "-");
      break;
    case 2:
      sprintf(buf + strlen(buf), "%s", "*");
      break;
    case 3:
      sprintf(buf + strlen(buf), "%s", "/");
      break;
    case 4:
      sprintf(buf + strlen(buf), "%s", "&&");
      break;
    case 5:
      sprintf(buf + strlen(buf), "%s", "||");
      break;
    case 6:
      sprintf(buf + strlen(buf), "%s", "==");
      break;
    case 7:
      sprintf(buf + strlen(buf), "%s", "!=");
      break;
  }
  return;
}

static inline void gen_blank()
{
  int x = rand() % 4;
  sprintf(buf + strlen(buf), "%*s", x, "");
}

static inline void gen_num(){
  int x = rand() % 1000;
  //int x = rand();
  sprintf(buf + strlen(buf), "%d", x);
  /*
  switch(choose(2))
  {
    case 0:
      sprintf(buf + strlen(buf), "%d", x);
      break;
    case 1:
      sprintf(buf+ strlen(buf), "%#x", x);
      break;
  }
  */
  return;
}

static char *cd_format =
"#include <stdio.h>\n"
"int main() { "
"  int result = %s; "
"  printf(\"%%d\", result); "
"  return 0; "
"}";

static int check_expr(char* buf, int x)
{
  //printf("test: \033[1m\033[33m%s\033[0m\n",buf);
  sprintf(code_buf, cd_format, buf);

  FILE* fp = fopen("/tmp/.code.c", "w");
  assert(fp != NULL);
  fputs(code_buf, fp);
  fclose(fp);

  //int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
  int ret = system("gcc -w /tmp/.code.c -o /tmp/.expr");
  if (ret != 0)
  {
      //printf("ret error: \033[1m\033[31m%s\033[0m\n", buf);//some strange error happens here, like 0x20-   0x1(compile error)
      return 0;
  }

  fp = popen("/tmp/.expr", "r");
  assert(fp != NULL);

  int result = 0x80000000;
  fscanf(fp, "%d", &result);
  pclose(fp);

  if (result == x)
  {
      //printf("result %d, error: \033[1m\033[31m%s\033[0m\n", result, buf);
      return 0;
  }
  //printf("answer: \033[1m\033[32m%d\033[0m\n",result);
  return 1;
}

static inline void gen_rand_expr(int flag) {
  char* judge_zero = buf;
	if(strlen(buf) > 0 && *( buf + strlen(buf) - 1) == '/')
  {
		judge_zero = buf + strlen(buf);
	}

  //if(strlen(buf) < 65536)
  //{
  switch(choose(5))
  {
    case 0:
          //printf("gen_rand_expr %d\n", 0);
      gen_blank();
      gen_rand_expr(0);
          //printf("cur buf %s in case 0\n", buf);
      break;
    case 1:
          //printf("gen_rand_expr %d\n", 1);
      gen_num();
          //printf("cur buf %s in case 1\n", buf);
      break;
    case 2:
          //printf("gen_rand_expr %d\n", 2);
      gen_rand_expr(0);
      gen_rand_op();
      gen_rand_expr(0);
          //printf("cur buf %s in case 2\n", buf);
      break;
    case 3:
          //printf("gen_rand_expr %d\n", 3);
      sprintf(buf + strlen(buf), "%s", "(");
      gen_rand_expr(0);
      sprintf(buf + strlen(buf), "%s", ")");
          //printf("cur buf %s in case 3\n", buf);
      break;
    case 4:
          //printf("gen_rand_expr %d\n", 4);
      /*
      if(choose(20) == 0)
        sprintf(buf + strlen(buf), "%s", "!");
      else
        sprintf(buf + strlen(buf), "%s", "");
      gen_rand_expr(0);
      */
      sprintf(buf + strlen(buf), "%s", "!");
      gen_num();

          //printf("cur buf %s in case 4\n", buf);
      break;
  }
  //}

  if(judge_zero != buf)
  {
    //printf("check 0\n");
    if(check_expr(judge_zero, 0) == 0)
    {
      //printf("divide 0: \033[1m\033[31m%s\033[0m\n", buf);
      memset(judge_zero, 0, sizeof(judge_zero));
      gen_rand_expr(0);
      return;
    }
  }

  if (flag == 1)
  {
    //printf("check expr\n");
      if(check_expr(buf, 0x80000000) == 0)
      {
        //printf("invalid expr, re-gen\n");
        memset(buf, 0, sizeof(buf));
        gen_rand_expr(1);
        return;
      }
  }
  //printf("%u %s\n", result, buf);

	return;
}
// TODO: if necessary, try to re-implement main function for better generation of expression

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;

  //FILE *ip = fopen("../../src/t.txt", "w");

  for (i = 0;  i < loop; i++) {
    buf[0]='\0';
    //printf("loop %d\n", i);

	  gen_rand_expr(1);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);

    //printf("\n");

    //fprintf(ip, "%u %s",result, buf);
    //fprintf(ip, "%c", '\n');

  }
  //fclose(ip);
  return 0;
}
