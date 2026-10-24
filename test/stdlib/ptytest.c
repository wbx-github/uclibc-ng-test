#define _XOPEN_SOURCE
#include <features.h>

#if defined __UCLIBC__ && !defined __UCLIBC_HAS_PTY__

/* stdlib.h declares ptsname() either way, but without the option it is not
   built, so this is a link error. */
#include <stdio.h>
int main(void)
{
    puts("needs UCLIBC_HAS_PTY");
    return 23;			/* SKIP */
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd;
    char *cp;

    fd=open("/dev/ptmx",O_NOCTTY|O_RDWR);
    if (fd < 0) {
	/* No devpts in the test rootfs, which is not something the library
	   can be blamed for -- and the old version returned failure here
	   without printing anything at all. */
	printf("/dev/ptmx: %s\n", strerror(errno));
	return 23;			/* SKIP */
    }
    cp=ptsname(fd);
    if (cp==NULL) {
	printf("ptsname: %s\n", strerror(errno));
	close(fd);
	return EXIT_FAILURE;
    }
    printf("ptsname %s\n",cp);
    close(fd);
    return EXIT_SUCCESS;
}

#endif
