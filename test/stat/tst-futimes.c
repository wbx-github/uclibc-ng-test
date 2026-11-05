/* futimes sets the times of an open file from two struct timeval.  It must
   leave the same timestamps as utimensat does for the same times, so the
   test does not depend on what the file system can store (microseconds,
   dates after 2038).  A null pointer sets both times to now.  */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

static const struct timeval tv[2] = {
	{ 1000000000, 250000 },
#if __SIZEOF_TIME_T__ > 4 || defined __USE_TIME_BITS64 || defined __UCLIBC_USE_TIME64__
	{ (time_t)4102444800LL, 500000 },	/* 2100-01-01 */
#else
	{ 2000000000, 500000 },
#endif
};

int main(void)
{
	char file[] = "/tmp/tst-futimes-XXXXXX";
	struct timespec ts[2], old[2] = { { 1, 0 }, { 1, 0 } };
	struct stat want, got;
	time_t before;
	int fd, err = 0;

	fd = mkstemp(file);
	if (fd < 0) {
		perror("mkstemp");
		return 1;
	}

	/* The reference: what utimensat makes of the same times.  */
	ts[0].tv_sec = tv[0].tv_sec;
	ts[0].tv_nsec = tv[0].tv_usec * 1000;
	ts[1].tv_sec = tv[1].tv_sec;
	ts[1].tv_nsec = tv[1].tv_usec * 1000;
	if (futimens(fd, ts) != 0 || fstat(fd, &want) != 0) {
		perror("futimens");
		err = 1;
		goto out;
	}

	futimens(fd, old);
	if (futimes(fd, tv) != 0) {
		printf("futimes: %s\n", strerror(errno));
		err = 1;
	} else if (fstat(fd, &got) != 0) {
		perror("fstat");
		err = 1;
	} else {
		if (got.st_atim.tv_sec != want.st_atim.tv_sec
		    || got.st_atim.tv_nsec != want.st_atim.tv_nsec) {
			printf("futimes: atime %lld.%09ld, want %lld.%09ld\n",
			       (long long)got.st_atim.tv_sec, (long)got.st_atim.tv_nsec,
			       (long long)want.st_atim.tv_sec, (long)want.st_atim.tv_nsec);
			err = 1;
		}
		if (got.st_mtim.tv_sec != want.st_mtim.tv_sec
		    || got.st_mtim.tv_nsec != want.st_mtim.tv_nsec) {
			printf("futimes: mtime %lld.%09ld, want %lld.%09ld\n",
			       (long long)got.st_mtim.tv_sec, (long)got.st_mtim.tv_nsec,
			       (long long)want.st_mtim.tv_sec, (long)want.st_mtim.tv_nsec);
			err = 1;
		}
	}

	/* NULL: both times become now.  */
	futimens(fd, old);
	before = time(NULL);
	if (futimes(fd, NULL) != 0 || fstat(fd, &got) != 0) {
		printf("futimes(NULL): %s\n", strerror(errno));
		err = 1;
	} else if (got.st_mtim.tv_sec < before - 1 || got.st_mtim.tv_sec > before + 5) {
		printf("futimes(NULL): mtime %lld, want about %lld\n",
		       (long long)got.st_mtim.tv_sec, (long long)before);
		err = 1;
	}

	/* An out-of-range tv_usec is refused.  */
	{
		struct timeval bad[2] = { { 1, 1000000 }, { 1, 0 } };

		errno = 0;
		if (futimes(fd, bad) != -1 || errno != EINVAL) {
			puts("futimes with tv_usec 1000000 did not fail with EINVAL");
			err = 1;
		}
	}

out:
	close(fd);
	unlink(file);
	return err;
}
