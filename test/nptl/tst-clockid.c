/*
 * Copyright (C) 2017 Sergey Korolev <s.korolev@ndmsystems.com>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int do_test (void)
{
    clockid_t clk;
    struct timespec ts;
    const int err = pthread_getcpuclockid(pthread_self(), &clk);

    if (err != 0) {
        errno = err;
        perror("pthread_getcpuclockid");
        return EXIT_FAILURE;
    }

    if (clock_gettime(clk, &ts) == -1) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }                                                                                                                                      

    printf("Thread time is %lu.%06lu.\n",
           ts.tv_sec,
           ts.tv_nsec / 1000);

    return EXIT_SUCCESS;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
