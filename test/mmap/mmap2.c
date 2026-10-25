/* mmap() with an offset that does not fit a 32-bit argument.
 *
 * The point of this test is the offset, not any particular file: on 32-bit
 * targets an offset near 4 GiB can only be expressed through the mmap2
 * syscall, which takes it in pages, so this exercises the conversion in the
 * C library.  It used to map the fixed physical address 0xfffff000 out of
 * /dev/mem, which is not a property of the library at all -- whether
 * anything mappable lives there depends on the platform, and the test was
 * switched off for arm, riscv32 and xtensa one architecture at a time.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

int main(void)
{
	char name[] = "mmap2.XXXXXX";
	long page = sysconf(_SC_PAGESIZE);
	unsigned char *map;
	off_t off;
	long i;
	int fd;

	/* The largest page aligned offset this off_t can express, with room for
	   one more page: just below 4 GiB where off_t is 64 bits wide, just
	   below 2 GiB where it is not.  Computed from the page size instead of
	   written down, so that it stays aligned where a page is not 4 KiB --
	   the conversion into pages is what is being tested, and that happens
	   either way.  */
	if (sizeof(off_t) >= 8)
		off = (off_t) ((0x100000000ULL / page - 1) * page);
	else
		off = (off_t) ((0x80000000UL / page - 2) * page);

	fd = mkstemp(name);
	if (fd == -1)
		FATAL;
	unlink(name);

	/* A hole, so nothing is allocated on a filesystem that has holes.  Not
	   every one does: ramfs on a target without an MMU keeps each file in a
	   single contiguous block of pages and refuses anything above MAX_ORDER
	   with EFBIG (fs/ramfs/file-nommu.c).  Fall back to a small offset
	   there -- the conversion into pages is still what gets tested, just
	   not at the far end of the range.  */
	if (ftruncate(fd, off + page) == -1) {
		if (errno != EFBIG && errno != EINVAL && errno != ENOSPC
		    && errno != ENOMEM && errno != EPERM)
			FATAL;
		printf("a file of %llu bytes is not possible here (%s), "
		       "using a small offset instead\n",
		       (unsigned long long) off + page, strerror(errno));
		off = 2 * page;
		if (ftruncate(fd, off + page) == -1)
			FATAL;
	}

	map = mmap(NULL, page, PROT_READ, MAP_PRIVATE, fd, off);
	if (map == MAP_FAILED)
		FATAL;
	printf("mapped %ld bytes at offset %llu to %p\n",
	       page, (unsigned long long) off, map);

	/* Reading a hole gives zeroes; anything else means the offset was not
	   passed through correctly.  */
	for (i = 0; i < page; i++)
		if (map[i] != 0) {
			printf("byte %ld of the hole is %02x, expected 00\n",
			       i, map[i]);
			munmap(map, page);
			close(fd);
			return 1;
		}

	if (munmap(map, page) == -1)
		FATAL;
	close(fd);
	return 0;
}
