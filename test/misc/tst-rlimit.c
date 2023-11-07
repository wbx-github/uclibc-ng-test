/* 
 * setrlimit/getrlimit/prlimit and setrlimit64/getrlimit64/prlimit64 functions 
 * test for uClibc.
 *
 * The prlimit64 function is not called directly in this test, because it is a
 * new function for uclibc and can cause build problems with uclibc-ng <= 1.0.44.
 * With _FILE_OFFSET_BITS == 64 the prlimit call is redirected to the prlimit64
 * call.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>

#define __ASSERT(x, f, l) \
	if (!(x)) { \
		fprintf(stderr, "%s: LINE %d: ASSERT: " #x "\n", f, l); \
		exit(1); \
	} 

#define ASSERT(x)  __ASSERT(x, __FILE__, __LINE__)

static int resources[] = {
	RLIMIT_CORE,
	RLIMIT_CPU,
	RLIMIT_DATA,
	RLIMIT_FSIZE,
	RLIMIT_NOFILE,
	RLIMIT_STACK,
	RLIMIT_AS
};

#define nresources	(sizeof (resources) / sizeof (resources[0]))
#define TEST_VAL	0x11223344


static void test_getrlimit(int rnum, rlim_t exp_cur, rlim_t exp_max)
{
	struct rlimit r;

	ASSERT(getrlimit(rnum, &r) == 0);
	ASSERT(r.rlim_cur == exp_cur);
	ASSERT(r.rlim_max == exp_max);
}

static void test_setrlimit(int rnum, rlim_t new_cur, rlim_t new_max)
{
	struct rlimit r = {.rlim_cur = new_cur, .rlim_max = new_max};

	ASSERT(setrlimit(rnum, &r) == 0);
}

static void test_prlimit_get(int rnum, rlim_t exp_cur, rlim_t exp_max)
{
	struct rlimit r;

	ASSERT(prlimit(0, rnum, NULL, &r) == 0);
	ASSERT(r.rlim_cur == exp_cur);
	ASSERT(r.rlim_max == exp_max);
}

static void test_prlimit_set(int rnum, rlim_t new_cur, rlim_t new_max)
{
	struct rlimit r = {.rlim_cur = new_cur, .rlim_max = new_max};
	ASSERT(prlimit(0, rnum, &r, NULL) == 0);
}

#if defined(__USE_LARGEFILE64)
static void test_getrlimit64(int rnum, rlim64_t exp_cur, rlim64_t exp_max)
{
	struct rlimit64 r;

	ASSERT(getrlimit64(rnum, &r) == 0);
	ASSERT(r.rlim_cur == exp_cur);
	ASSERT(r.rlim_max == exp_max);
}

static void test_setrlimit64(int rnum, rlim64_t new_cur, rlim64_t new_max)
{
	struct rlimit64 r = {.rlim_cur = new_cur, .rlim_max = new_max};

	ASSERT(setrlimit64(rnum, &r) == 0);
}
#endif

int main(void)
{
	int rnum = -1;
	struct rlimit rlim;
	int i, ret;

	/* Find a resource with hard limit set to infinity */
	for (i = 0; i < nresources; ++i) {
		ret = getrlimit(resources[i], &rlim);
		if ((!ret) && (rlim.rlim_max == RLIM_INFINITY)) {
			rnum = resources[i];
			break;
		}
	}

	/* Can't continue, return unsupported */
	if (rnum == -1)
		return 23;

	/* Test cases */
	test_setrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_getrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_setrlimit(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_getrlimit(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_prlimit_get(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_prlimit_set(rnum, TEST_VAL, RLIM_INFINITY);
	test_prlimit_get(rnum, TEST_VAL, RLIM_INFINITY);
	test_getrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_prlimit_set(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_getrlimit(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_prlimit_get(rnum, RLIM_INFINITY, RLIM_INFINITY);

#if defined(__USE_LARGEFILE64)
	/* Check setrlim64 and getrlim64 in 32-bit offset LFS environment */
	test_setrlimit64(rnum, TEST_VAL, RLIM64_INFINITY);
	test_getrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_getrlimit64(rnum, TEST_VAL, RLIM64_INFINITY);
	test_setrlimit64(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
	test_prlimit_get(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_getrlimit64(rnum, RLIM64_INFINITY, RLIM64_INFINITY);

	test_setrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_getrlimit64(rnum, TEST_VAL, RLIM64_INFINITY);
	test_prlimit_set(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_getrlimit64(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
#endif


#if defined(__USE_LARGEFILE64) && _FILE_OFFSET_BITS == 64
	/* Check setrlim64/getrlim64/prlimit64 in 64-bit offset environment */
	test_setrlimit64(rnum, TEST_VAL, RLIM64_INFINITY);
	test_getrlimit64(rnum, TEST_VAL, RLIM64_INFINITY);
	test_getrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_setrlimit64(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
	test_prlimit_get(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
	test_getrlimit(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_prlimit_set(rnum, TEST_VAL, RLIM64_INFINITY);
	test_prlimit_get(rnum, TEST_VAL, RLIM64_INFINITY);
	test_prlimit_set(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
	test_getrlimit64(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
	test_getrlimit(rnum, RLIM_INFINITY, RLIM_INFINITY);

	test_setrlimit(rnum, TEST_VAL, RLIM_INFINITY);
	test_getrlimit64(rnum, TEST_VAL, RLIM64_INFINITY);
	test_prlimit_set(rnum, RLIM_INFINITY, RLIM_INFINITY);
	test_getrlimit64(rnum, RLIM64_INFINITY, RLIM64_INFINITY);
#endif

	return 0;
}
