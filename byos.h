#ifndef _BYOS_H
#define _BYOS_H

typedef struct echo_d {
  char *arg;
} echo_d;

typedef struct forx_d {
  char *pathname;
  char **argv;
  // The smallest i such that argv[i]==NULL marks the end.
  // IOW ready to be passed to an appropriate exec?? syscall.
} forx_d;

struct cmd;

typedef struct list_d {
  int n;  // >=0, how many elements in the array below
  struct cmd *cmds;
} list_d;

typedef enum cmdtype { ECHO, FORX, LIST } cmdtype;

typedef struct cmd {
  char *redir_stdout;  // pathname to redirect to; if NULL, don't redirect
  enum cmdtype type;
  union {
    struct echo_d echo;  // when type==ECHO
    struct forx_d forx;  // when type==FORX
    struct list_d list;  // when type==LIST
  } data;
} cmd;


int interp(const struct cmd *);

#endif
