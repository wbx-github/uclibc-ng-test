/* make sure we handle argv[0] == NULL */

#include <unistd.h>

int main(int argc, char *argv[])
{
	/* since Linux https://github.com/torvalds/linux/commit/dcd46d897adb70d63e025f175a00a89797d31a43
	 * kernel forces an empty first arg if execve is called
	 * with argv == NULL.
	 * so we need to handle argc == 1 for newer kernel as well
	 */
	if (argc == 0 || argc == 1)
		return 0;

	char *exec_argv[1], *exec_envp[1];
	exec_argv[0] = exec_envp[0] = NULL;
	return execve("./exec-null", exec_argv, exec_envp);
}
