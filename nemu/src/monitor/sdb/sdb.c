/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"
#include "common.h"
#include "debug.h"
#include "memory/paddr.h"
#include "utils.h"
#include <cpu/cpu.h>
#include <errno.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}
static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);

bool parse_ull(const char *s, uint64_t *out);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "[N] \n  Execute N instructions (default: 1), then stop", cmd_si},
    {"info", "[r | <reg>]\n  display register info", cmd_info},
    {"x", "[N] [Addr]  display memory data, N is number of Byte", cmd_x}};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) { /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_x(char *args) {
  char *num_str = strtok(args, " ");
  if (num_str == NULL)
    return 0;

  uint64_t num;
  parse_ull(num_str, &num);

  char *addr_str = strtok(NULL, " ");
  if (addr_str == NULL)
    return 0;

  uint64_t tmp;
  parse_ull(addr_str, &tmp);
  paddr_t addr = (paddr_t)tmp;

  for (int i = 0; i < num; ++i) {
    printf("0x%08x: ", addr);
    for (int j = 0; j < 4; j++) {
      printf("%02x  ", paddr_read(addr, 1));
      addr += 1;
    }
    printf("\n");
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(args, " ");
  if (arg == NULL)
    return 0;

  if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else {
    isa_target_reg_display(arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  uint64_t inst_n = 1;
  if (arg != NULL) {
    if (!parse_ull(arg, &inst_n)) {
      return 0;
    }
  }
  cpu_exec(inst_n);
  return 0;
}

bool parse_ull(const char *s, uint64_t *out) {
  if (!s || !*s) {
    printf("N must be number\n");
    return false;
  }

  char *end;
  errno = 0;

  // base = 0 自动识别 0x/0 前缀
  uint64_t v = strtoull(s, &end, 0);

  // 如果没有解析到任何字符
  if (end == s) {
    printf("N must be number\n");
    return false;
  }

  // 如果后面还有非数字字符
  if (*end != '\0') {
    printf("N must be number\n");
    return false;
  }

  // 检查溢出
  if (errno == ERANGE) {
    printf("N is too large\n");
    return false;
  }

  *out = v;
  return true;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          // Command returns negative value to indicate exit
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
