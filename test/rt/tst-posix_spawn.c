#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

extern char **environ;

void run_cmd(char *cmd)
{
    pid_t pid;
    posix_spawnattr_t attrs;
    posix_spawn_file_actions_t actions;
    sigset_t defsignals;
    char *argv[] = {"sh", "-c", cmd, NULL};
    int status;

    sigemptyset(&defsignals);
    sigaddset(&defsignals, SIGTERM);
    sigaddset(&defsignals, SIGCHLD);
    sigaddset(&defsignals, SIGPIPE);

    posix_spawnattr_init(&attrs);
    posix_spawnattr_setflags(&attrs, POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGDEF);
    posix_spawnattr_setpgroup(&attrs, 0);
    posix_spawnattr_setsigdefault(&attrs, &defsignals);

    printf("Run command: %s\n", cmd);
    status = posix_spawn(&pid, "/bin/sh", &actions, &attrs, argv, environ);
    if (status == 0) {
        printf("Child pid: %i\n", pid);
        if (waitpid(pid, &status, 0) != -1) {
            printf("Child exited with status %i\n", status);
        } else {
            perror("waitpid");
        }
    } else {
        printf("posix_spawn: %s\n", strerror(status));
    }
}

int main(int argc, char* argv[])
{
    run_cmd(argv[1]);
    return 0;
}
