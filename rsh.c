#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

/* Allowed commands exactly as given */
char *allowed[N] = {
    "cp","touch","mkdir","ls","pwd","cat",
    "grep","chmod","diff","cd","exit","help"
};

int isAllowed(const char *cmd) {
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void print_help() {
    printf("The allowed commands are:\n");
    printf("1: cp\n");
    printf("2: touch\n");
    printf("3: mkdir\n");
    printf("4: ls\n");
    printf("5: pwd\n");
    printf("6: cat\n");
    printf("7: grep\n");
    printf("8: chmod\n");
    printf("9: diff\n");
    printf("10: cd\n");
    printf("11: exit\n");
    printf("12: help\n");
}

int main() {

    char line[256];

    while (1) {

        /* prompt goes to stderr */
        fprintf(stderr,"rsh>");

        /* read line */
        if (fgets(line, 256, stdin) == NULL)
            continue;

        /* skip empty lines */
        if (strcmp(line, "\n") == 0)
            continue;

        /* strip newline */
        line[strlen(line)-1] = '\0';

        /* -------- TOKENIZE INTO argv ------- */
        char *argv[21];  // max 20 args + NULL
        int argc = 0;

        char *tok = strtok(line, " \t");
        while (tok != NULL && argc < 20) {
            argv[argc++] = tok;
            tok = strtok(NULL, " \t");
        }
        argv[argc] = NULL;

        if (argc == 0)
            continue;

        /* name of command */
        char *cmd = argv[0];

        /* --------- handle built-ins -------- */

        /* exit */
        if (strcmp(cmd, "exit") == 0) {
            return 0;
        }

        /* help */
        if (strcmp(cmd, "help") == 0) {
            print_help();
            continue;
        }

        /* cd */
        if (strcmp(cmd, "cd") == 0) {

            if (argc > 2) {
                /* EXACT message required */
                printf("-rsh: cd: too many arguments\n");
                continue;
            }

            if (argc == 1) {
                /* cd to HOME */
                char *home = getenv("HOME");
                if (home != NULL)
                    chdir(home);
            } else {
                /* cd <path> */
                chdir(argv[1]);
            }

            continue;
        }

        /* ---------- allowed external commands ---------- */
        if (isAllowed(cmd) && 
            (strcmp(cmd,"cp") == 0 ||
            strcmp(cmd,"touch") == 0 ||
            strcmp(cmd,"mkdir") == 0 ||
            strcmp(cmd,"ls") == 0 ||
            strcmp(cmd,"pwd") == 0 ||
            strcmp(cmd,"cat") == 0 ||
            strcmp(cmd,"grep") == 0 ||
            strcmp(cmd,"chmod") == 0 ||
            strcmp(cmd,"diff") == 0))
        {
	    pid_t pid;
            int status;

            int ret = posix_spawnp(&pid, cmd, NULL, NULL, argv, environ);
            if (ret != 0) {
                printf("NOT ALLOWED!\n");
                continue;
            }

            waitpid(pid, &status, 0);
            continue;
        }

        /* ---------- everything else: NOT ALLOWED ---------- */
        printf("NOT ALLOWED!\n");
    }

    return 0;
}

