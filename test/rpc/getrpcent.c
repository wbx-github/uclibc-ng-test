#include <netdb.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
#if defined(__UCLIBC_HAS_RPC__)
	struct rpcent *ent;

	while ((ent = getrpcent()) != NULL) {
		printf("%s: %i", ent->r_name, ent->r_number);
		while (ent->r_aliases[0])
			printf(" %s", *ent->r_aliases++);
		printf("\n");
	}

	endrpcent();
#endif
	return 0;
}
