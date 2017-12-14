#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main() {
	int pid;

	pid = syscall(SYS_getpid);
   	if (pid > 0) {
		printf("syscall(SYS_getpid) says %d\n", pid);
     		return 0;
	}

	return 1;
}
