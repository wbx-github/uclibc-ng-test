#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/utsname.h>

int main() {
	int ret;
	struct utsname name;

	ret = syscall(SYS_uname, &name);
   	if (ret == 0) {
		printf("syscall(SYS_uname) says %s-%s\n", name.sysname,
			name.release);
     		return 0;
	}

	return 1;
}
