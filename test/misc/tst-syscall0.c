#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef SYS_getpid
#define _SYS_call0 SYS_getxpid
#else
#define _SYS_call0 SYS_getpid
#endif

int main() {
	int pid;

	pid = syscall(_SYS_call0);
   	if (pid > 0) {
		printf("syscall(SYS_getpid) says %d\n", pid);
     		return 0;
	}

	return 1;
}
