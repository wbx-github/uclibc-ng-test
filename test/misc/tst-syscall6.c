#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <linux/fs.h> /* for RWF_HIPRI */

int main()
{
#if defined SYS_preadv2 && defined SYS_pwritev2 && defined RWF_HIPRI
	char tmp[] = "/tmp/tst-preadv2-XXXXXX";
	int fd;
	struct iovec iov[2];
	char *str0 = "hello ";
	char *str1 = "world\n";
	char input[16];
	int nio;

	fd = mkstemp (tmp);
	if (fd == -1) {
		puts ("mkstemp failed");
		return 1;
	}

	iov[0].iov_base = str0;
	iov[0].iov_len = strlen(str0);
	iov[1].iov_base = str1;
	iov[1].iov_len = strlen(str1) + 1; /* null terminator */

	nio = syscall(SYS_pwritev2, fd, iov, 2, 0, 0, RWF_DSYNC);

	if (nio <= 0) {
		puts ("failed to write to fd");
		return 1;
	}

	/* Read in the second string into the first buffer */
	iov[0].iov_base = input;
	iov[0].iov_len = strlen(str1) + 1; /* null terminator */
	nio = syscall(SYS_preadv2, fd, iov, 1, strlen(str0), 0, RWF_HIPRI);
	if (nio <= 0) {
		printf ("failed to read fd %d\n", nio);
		return 1;
	}

	if (strncmp(iov[0].iov_base, iov[1].iov_base, strlen(str1)) == 0)
		printf ("syscall(SYS_preadv2) read %s", (char *) iov[0].iov_base);

	if (close(fd) != 0) {
		puts ("failed to close read fd");
		return 1;
	}

	if (unlink(tmp) != 0) {
		puts ("failed to unlink file");
		return 1;
	}

	return 0;
#else
  return 23;
#endif
}
