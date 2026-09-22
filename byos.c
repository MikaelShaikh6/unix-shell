#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "byos.h"

int interp(const struct cmd *c)
{
    if (c == NULL) {
         return 0;
    }

    int saved_out = -1;

    // redirect output if needed
    if (c->redir_stdout != NULL) {
        saved_out = dup(1);
        if (saved_out == -1) {
            perror("dup failed: ");
            return 1;
        }

        if (fcntl(saved_out, F_SETFD, FD_CLOEXEC) == -1) {
            perror("fcntl failed: ");
            close(saved_out);
            return 1;
        }

        int tmp_fd = open(c->redir_stdout, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (tmp_fd == -1) {
            perror("error opening file: ");
            close(saved_out);
            return 1;
        }

        if (dup2(tmp_fd, 1) == -1) {
            perror("dup2 failed: ");
            close(tmp_fd);
            close(saved_out);
            return 1;
        }

        close(tmp_fd);
    }

    int result = 0;

    // echo case
    if (c->type == ECHO) {
        const char *arg = c->data.echo.arg;
        if (write(STDOUT_FILENO, arg, strlen(arg)) == -1) {
            result = 1;
        }
    }
    // forx case
    else if (c->type == FORX) {
        char *pathname = c->data.forx.pathname;
        char **argv = c->data.forx.argv;

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed!");
            exit(1);
        }
        else if (pid == 0) {
            execvp(pathname, argv);
            perror("exec syscall failed!");
            _exit(127);
        }
        else {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("wait failed: ");
                result = 1;
            }
            else if (WIFEXITED(status)) {
                result = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status)) {
                result = 128 + WTERMSIG(status);
            }
            else {
                result = 1;
            }
        }
    }
    // list case
    else if (c->type == LIST) {
        for (int i = 0; i < c->data.list.n; i++) {
            result = interp(&c->data.list.cmds[i]);
        }
    }

    if (c->redir_stdout != NULL && saved_out != -1) {
        if (dup2(saved_out, 1) == -1) {
            perror("dup2 failed: ");
        }
        close(saved_out);
    }

    return result;
}
