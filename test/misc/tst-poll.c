/* Check poll's per-entry count and events independent of the request mask.
   Licensed under the LGPL v2.1, see COPYING.LIB.  */

#include <features.h>

#if defined(__UCLIBC__) && !defined(__UCLIBC_HAS_SOCKET__)
int
main(void)
{
    return 0;
}
#else

#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

static int failures;

static void
check(const char *name, int ok)
{
    if (!ok) {
        printf("FAIL: %s\n", name);
        failures++;
    }
}

int
main(void)
{
    struct pollfd fds[2];
    int sv[2], p[2];
    int fd, ret;

    /* Leave data queued so the socket is both readable and writable. */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1
        || write(sv[1], "x", 1) != 1) {
        perror("socketpair/write");
        return 1;
    }

    /* Count ready array entries, not the individual event bits. */
    fds[0].fd = sv[0];
    fds[0].events = POLLIN | POLLOUT;
    fds[0].revents = 0;
    ret = poll(fds, 1, 0);
    check("readable and writable entry counted once",
          ret == 1 && fds[0].revents == (POLLIN | POLLOUT));

    /* Two entries for the same descriptor must each be counted. */
    fds[0].events = POLLIN;
    fds[1] = fds[0];
    ret = poll(fds, 2, 0);
    check("duplicate entries counted separately",
          ret == 2 && fds[0].revents == POLLIN
          && fds[1].revents == POLLIN);

    /* Each entry must keep its own event mask. */
    fds[1].events = POLLOUT;
    ret = poll(fds, 2, 0);
    check("duplicate entries keep their individual event masks",
          ret == 2 && fds[0].revents == POLLIN
          && fds[1].revents == POLLOUT);

    close(sv[0]);
    close(sv[1]);

    if (pipe(p) == -1) {
        perror("pipe");
        return 1;
    }

    /* Report hangup even when no events were requested. */
    close(p[1]);
    fds[0].fd = p[0];
    fds[0].events = 0;
    fds[0].revents = 0;
    ret = poll(fds, 1, 0);
    check("hangup reported with events zero",
          ret == 1 && fds[0].revents == POLLHUP);

    /* An invalid descriptor must also be reported with events zero. */
    fd = p[0];
    close(fd);
    fds[0].fd = fd;
    fds[0].events = 0;
    ret = poll(fds, 1, 0);
    check("invalid descriptor reported with events zero",
          ret == 1 && fds[0].revents == POLLNVAL);

    /* Ignore negative descriptors and clear any stale result bits. */
    fds[0].fd = -2;
    fds[0].events = POLLIN | POLLOUT;
    fds[0].revents = POLLIN;
    ret = poll(fds, 1, 0);
    check("negative descriptor ignored and revents cleared",
          ret == 0 && fds[0].revents == 0);

    return failures != 0;
}
#endif
