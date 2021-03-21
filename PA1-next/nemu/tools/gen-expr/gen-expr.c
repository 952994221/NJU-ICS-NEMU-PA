#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = "";
static char code_buf[65536];
static char *code_format =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned result = %s; "
    "  printf(\"%%u\", result); "
    "  return 0; "
    "}";
// TODO: implement these functions: choose, gen_rand_op, gen_num, gen_rand_expr
static inline uint32_t choose(uint32_t n)
{
  return rand() % n;
}

static inline void gen_rand_op()
{
  switch (choose(8))
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

static inline void gen_num()
{
  int x = rand() % 1000;
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

static int check_expr(char *buf, int x)
{
  sprintf(code_buf, cd_format, buf);

  FILE *fp = fopen("/tmp/.code.c", "w");
  assert(fp != NULL);
  fputs(code_buf, fp);
  fclose(fp);

  //int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
  int ret = system("gcc -w /tmp/.code.c -o /tmp/.expr");
  if (ret != 0)
  {
    return 0;
  }

  fp = popen("/tmp/.expr", "r");
  assert(fp != NULL);

  int result = 0x80000000;
  fscanf(fp, "%d", &result);
  pclose(fp);

  if (result == x)
  {
    return 0;
  }
  return 1;
}

static inline void gen_rand_expr(int flag)
{
  char *judge_zero = buf;
  if (strlen(buf) > 0 && *(buf + strlen(buf) - 1) == '/')
  {
    judge_zero = buf + strlen(buf);
  }

  switch (choose(5))
  {
  case 0:
    gen_blank();
    gen_rand_expr(0);
    break;
  case 1:
    gen_num();
    break;
  case 2:
    gen_rand_expr(0);
    gen_rand_op();
    gen_rand_expr(0);
    break;
  case 3:
    sprintf(buf + strlen(buf), "%s", "(");
    gen_rand_expr(0);
    sprintf(buf + strlen(buf), "%s", ")");
    break;
  case 4:
    /*
      if(choose(20) == 0)
        sprintf(buf + strlen(buf), "%s", "!");
      else
        sprintf(buf + strlen(buf), "%s", "");
      gen_rand_expr(0);
      */
    sprintf(buf + strlen(buf), "%s", "!");
    gen_num();
    break;
  }
  //}

  if (judge_zero != buf)
  {
    if (check_expr(judge_zero, 0) == 0)
    {
      memset(judge_zero, 0, sizeof(judge_zero));
      gen_rand_expr(0);
      return;
    }
  }

  if (flag == 1)
  {
    if (check_expr(buf, 0x80000000) == 0)
    {
      memset(buf, 0, sizeof(buf));
      gen_rand_expr(1);
      return;
    }
  }
  return;
}
// TODO: if necessary, try to re-implement main function for better generation of expression

int main(int argc, char *argv[])
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1)
  {
    sscanf(argv[1], "%d", &loop);
  }
  int i;

  for (i = 0; i < loop; i++)
  {
    buf[0] = '\0';

    gen_rand_expr(1);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
