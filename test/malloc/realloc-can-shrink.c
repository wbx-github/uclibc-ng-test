/* make sure that realloc() can properly shrink buffers */

#include <stdlib.h>

#define BUFFER (1 << 19)

int main(int argc, char *argv[])
{
	int count = 20;
	char *ptr = NULL;
	while (count--) {
		ptr = realloc(ptr, BUFFER);
		ptr = realloc(ptr, 1);
	}
	free(ptr);
	return 0;
}
