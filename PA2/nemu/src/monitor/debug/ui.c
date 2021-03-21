#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include <dirent.h> // for c language, file directory operations (use 'man opendir' for more information)
#include <unistd.h> // for c language, get work path (use 'man getcwd' for more information)
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

static int cmd_pwd(char *);
static int cmd_echo(char *); // define functions

void cpu_exec(uint64_t);
void isa_reg_display();
uint32_t expr(char *, bool *);
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
    static char *line_read = NULL;

    if (line_read)
    {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read)
    {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char *args)
{
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char *args)
{
    return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_ls(char *args);

static int cmd_info(char *args);

static int cmd_p(char *args);

static int cmd_x(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct
{
    char *name;
    char *description;
    int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"info", "Print information of register and watchpoint", cmd_info},
    {"si", "Execute for N steps, if N is not given, exec_once", cmd_si},
    {"x", "Scan Memory from start, for total N bytes", cmd_x},
    {"p", "Compute the value of an expression", cmd_p},
    {"echo", "Print the characters given by user", cmd_echo}, // add by wuran
    {"pwd", "Print current work path", cmd_pwd},              // add by wuran
    {"ls", "List all files in given path", cmd_ls},
    {"w", "Add a new watchpoint", cmd_w},
    {"d", "Delete a watchpoint or all watchpoints", cmd_d}};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0])) // number of commands

static int cmd_help(char *args)
{
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL)
    {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++)
        {
            printf("\033[1m\033[33m [%s]\033[0m - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    }
    else
    {
        for (i = 0; i < NR_CMD; i++)
        {
            if (strcmp(arg, cmd_table[i].name) == 0)
            {
                printf("\033[1m\033[33m [%s]\033[0m - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        printf("\033[1m\033[31mUnknown command\033[0m '%s'\n", arg);
    }
    return 0;
}

static int cmd_echo(char *args)
{
    // char * arg = strtok(args, " ");
    if (args != NULL)
        printf("%s\n", args);
    else
        printf("\n");
    return 0;
}

static int cmd_pwd(char *args)
{
    char buf[256];
    if (getcwd(buf, 256) != 0)
        printf("\033[1m\033[33mcurrent work path: \033[0m%s\n", buf);
    return 0;
}

static int cmd_si(char *args)
{
    int num = -1;
    if (args == NULL)
        num = 1;
    else
        sscanf(args, "%d", &num);
    if (num > 0)
    {
        cpu_exec(num);
        printf("\033[1m\033[32mExecute for %d steps finished\033[0m\n", num);
    }
    else
        printf("\033[1m\033[31mPlease enter a positive number\033[0m\n");
    return 0;
}

static int cmd_ls(char *args)
{
    char buf[256];
    if (args == NULL)
    {
        if (getcwd(buf, 256) == 0)
        {
            printf("\033[1m\033[31mInvalid path\033[0m\n");
            return 0;
        }
    }
    else
        sscanf(args, "%s", buf);
    DIR *dir_p;
    struct dirent *entry;
    if ((dir_p = opendir(buf)) == NULL)
    {
        printf("\033[1m\033[31mInvalid path\033[0m\n");
        return 0;
    }
    else
    {
        printf("\033[1m\033[33mList the file in path:\033[0m %s\n", buf);
        while ((entry = readdir(dir_p)) != NULL)
        {
            printf("%-s\t", entry->d_name);
        }
        printf("\n");
    }
    return 0;
}

static int cmd_info(char *args)
{
  if(args == NULL)
  {
    printf("\033[1m\033[31mToo few arguments\033[0m\n");
    return 0;
  }
    if (strcmp(args, "r") == 0)
    {
        isa_reg_display();
    }
    else if (strcmp(args, "pc") == 0)
    {
        printf("\033[1m\033[33m[pc]:\033[0m\taddress: %#x, value: %#x\n", cpu.pc, paddr_read(cpu.pc, 4));
    }
    else if (strcmp(args, "w") == 0)
    {
      print_wp();
    }
    else
        printf("\033[1m\033[31mInvalid arguments\033[0m\n");
    return 0;
}

static int cmd_p(char *args)
{
    if (args != NULL)
    {
        bool success;
        uint32_t ans = expr(args, &success);
        if (success)
            printf("\033[1m\033[33mResult is\033[0m %#xH \033[1m\033[33min hex, is\033[0m %d \033[1m\033[33min dec, is\033[0m %u \033[1m\033[33min unsigned format\033[0m\n", ans, ans, ans);
        else
        {
            printf("\033[1m\033[31mPlease check you expression\033[0m\n");
            return 0;
        }
    }
    else
        printf("\n");
    return 0;
}

static int cmd_x(char *args)
{
  if(args == NULL)
  {
    printf("\033[1m\033[31mToo few arguments\033[0m\n");
    return 0;
  }
    char *args_end = args + strlen(args);
    char *str1 = strtok(args, " ");
    if (str1 == NULL)
    {
        printf("\033[1m\033[31mInvalid arguments\033[0m\n");
        return 0;
    }
    args = str1 + strlen(str1) + 1;
    if (args >= args_end || args == NULL)
    {
        printf("\033[1m\033[31mInvalid arguments\033[0m\n");
        return 0;
    }
    int n = 0;
    sscanf(str1, "%d", &n);
    bool success;
    uint32_t addr = expr(args, &success);
    if (success == false)
    {
        printf("\033[1m\033[31mPlease check you expression\033[0m\n");
        return 0;
    }
    int i;
    for (i = 0; i < n; i++)
    {
        printf("%#x:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n", addr, paddr_read(addr, 1), paddr_read(addr + 1, 1), paddr_read(addr + 2, 1), paddr_read(addr + 3, 1));
        addr += 4;
    }
    return 0;
}

static int cmd_w(char *args)
{
  if(args == NULL)
  {
    printf("\033[1m\033[31mToo few arguments\033[0m\n");
    return 0;
  }
  bool success;
  int val = expr(args, &success);
  if(success == false)
  {
    printf("Invalid arguments\n");
    return 0;
  }
  new_wp(args, val);
  return 0;
}

static int cmd_d(char *args)
{
  if(args != NULL)
  {
    int n = 0;
    sscanf(args, "%d", &n);
    free_wp(n);
  }
  else
    delete_all_wp();
  return 0;
}

void ui_mainloop(int is_batch_mode)
{
    if (is_batch_mode)
    {
        cmd_c(NULL);
        return;
    }
    char *str;
    for (; (str = rl_gets()) != NULL;)
    {
        char *str_end = str + strlen(str);

        /* extract the first token as the command */
        char *cmd = strtok(str, " ");
        if (cmd == NULL)
        {
            continue;
        }

        /* treat the remaining string as the arguments,
     * which may need further parsing
     */
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end)
        {
            args = NULL;
        }

#ifdef HAS_IOE
        extern void sdl_clear_event_queue(void);
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++)
        {
            if (strcmp(cmd, cmd_table[i].name) == 0)
            {
                if (cmd_table[i].handler(args) < 0)
                {
                    return;
                }
                break;
            }
        }

        if (i == NR_CMD)
        {
            printf("\033[1m\033[31mUnknown command\033[0m '%s'\n", cmd);
        }
    }
}
