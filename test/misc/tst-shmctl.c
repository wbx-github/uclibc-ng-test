#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

struct timespec ts = {
  .tv_sec = 3468960000,  // 2075-12-05 Destination timestamp
  .tv_nsec = 0
};

void print_shmid_ds(struct shmid_ds *buf) {
    printf("shm_perm.uid: %d \n", buf->shm_perm.uid);
    printf("shm_perm.gid: %d \n", buf->shm_perm.gid);
    printf("shm_perm.cuid: %d \n", buf->shm_perm.cuid);
    printf("shm_perm.cgid: %d \n", buf->shm_perm.cgid);
    printf("shm_perm.mode: %o \n", buf->shm_perm.mode);
    printf("shm_segsz: %lu \n", buf->shm_segsz);
    printf("shm_lpid: %d \n", buf->shm_lpid);
    printf("shm_cpid: %d \n", buf->shm_cpid);
    printf("shm_nattch: %lu \n", buf->shm_nattch);
    printf("shm_atime: %s", buf->shm_atime ? ctime(&buf->shm_atime) : "Not set\n");
    printf("shm_dtime: %s", buf->shm_dtime ? ctime(&buf->shm_dtime) : "Not set\n");
    printf("shm_ctime: %s\n", ctime(&buf->shm_ctime));
}

int main() {

    struct timespec ts_init, ts_final;

    // Save system time
    if (clock_gettime(CLOCK_REALTIME, &ts_init) == -1) {
        perror("Error getting time");
        return 1;
    }

    if (clock_settime(CLOCK_REALTIME, &ts) == -1) { // Set the time to after 2038
        perror("Error setting time");
        return 1;
    }

    key_t key = ftok(".", 'S');
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct shmid_ds buf;
    if (shmctl(shmid, IPC_STAT, &buf) == -1) {
        perror("shmctl");
        exit(1);
    }

    printf("Shared Memory Segment Info:\n");
    print_shmid_ds(&buf);

    // Change to new permissions
    buf.shm_perm.mode = 0600;
    
    if (shmctl(shmid, IPC_SET, &buf) == -1) {
        perror("shmctl IPC_SET failed");
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    if ((buf.shm_ctime - ts.tv_sec > 60) || (ts.tv_sec - buf.shm_ctime > 60)) {
        printf("\nShmctl get a error time! \n");
        exit(EXIT_FAILURE);
    }

    printf("Shared Memory Segment Info:\n");
    print_shmid_ds(&buf);

    shmctl(shmid, IPC_RMID, NULL);

    // Restore system time
    clock_gettime(CLOCK_REALTIME, &ts_final);
    ts_init.tv_sec = ts_init.tv_sec + ts_final.tv_sec - ts.tv_sec;
    clock_settime(CLOCK_REALTIME, &ts_init);

    return 0;
}
