#include "nemu.h"
#include <stdlib.h>
#include <string.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h> // for c languare, regularized expressions

enum
{
    TK_NUMBER = 256,
    TK_HEX,
    TK_MUL,
    TK_DIV,
    TK_ADD,
    TK_SUB,
    TK_EQ,
    TK_LEFT_PAR,
    TK_RIGHT_PAR,
    TK_NOTYPE,
    TK_LETTER
};

static struct rule
{
    char *regex;
    int token_type;
} rules[] = {
    {"\\b[0-9]+\\b", TK_NUMBER},
    {"\\b0[xX][0-9a-fA-F]+\\b", TK_HEX},
    {"\\+", TK_ADD},
    {"-", TK_SUB},
    {"\\*", TK_MUL},
    {"/", TK_DIV},
    {"\\(", TK_LEFT_PAR},
    {"\\)", TK_RIGHT_PAR},
    {" +", TK_NOTYPE},
    {"==", TK_EQ},
    {"\\b[a-zA-Z]+\\b", TK_LETTER}};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {}; // regex_t store number of regexs

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++)
    {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        // regcomp(regex_t *preg, const char * regex, int cflags)，description of function regcomp
        if (ret != 0)
        {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token
{
    int type;
    char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0; // number of regex tokens

    while (e[position] != '\0')
    {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++)
        {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
            {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                    i, rules[i].regex, position, substr_len, substr_len, substr_start);
                position += substr_len;

                if (rules[i].token_type == TK_NOTYPE)
                    break;
                else
                {
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                    tokens[nr_token].type = rules[i].token_type;
                    nr_token++;
                    break;
                }
            }
        }
        if (i == NR_REGEX)
        {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }
    return true;
}

bool check_parentheses(int start, int end)
{
    if (tokens[start].type != TK_LEFT_PAR || tokens[end].type != TK_RIGHT_PAR)
        return false;
    int i, count = 0, pos = 0, flag = 0;
    for (i = start; i <= end; i++)
    {
        if (tokens[i].type == TK_LEFT_PAR)
            count++;
        if (tokens[i].type == TK_RIGHT_PAR)
            count--;
        if (flag == 0 && count == 0)
        {
            pos = i;
            flag = 1;
        }
    }
    if (count != 0)
        Assert(0, "Please check you expression\n");
    if (pos == end)
        return true;
    else
        return false;
}

int str2int(char *str, int type)
{
    int ret = 0;
    if (type == TK_NUMBER)
        sscanf(str, "%d", &ret);
    if (type == TK_HEX)
        sscanf(str, "%x", &ret);
    return ret;
}

int eval(int start, int end)
{
    if (start > end)
        Assert(0, "Please check you expression\n");
    else if (start == end)
        return str2int(tokens[start].str, tokens[start].type);
    else if (check_parentheses(start, end) == true)
        return eval(start + 1, end - 1);
    else
    {
        int i, op = 0, pos = 0, val1, val2;
        for (i = start; i <= end; i++)
        {
            if (tokens[i].type == TK_LEFT_PAR)
            {
                do
                    i++;
                while (tokens[i].type != TK_RIGHT_PAR);
                continue;
            }
            if (tokens[i].type / 2 >= op)
            {
                op = tokens[i].type / 2;
                pos = i;
            }
        }
        val1 = eval(start, pos - 1);
        val2 = eval(pos + 1, end);
        switch (tokens[pos].type)
        {
        case TK_ADD:
            return val1 + val2;
        case TK_SUB:
            return val1 - val2;
        case TK_MUL:
            return val1 * val2;
        case TK_DIV:
            return val1 / val2;
        case TK_EQ:
            return val1 == val2;
        default:
            Assert(0, "Please check you expression\n");
        }
    }
}

uint32_t expr(char *e, bool *success)
{
    if (make_token(e) == false)
    {
        *success = false;
        return 0;
    }
    *success = true;
    return eval(0, nr_token - 1);
}
