#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

struct timespec ts = {
  .tv_sec = 3468960000,  // 3468960000 2075-12-05 Destination timestamp
  .tv_nsec = 0
};

void print_semid_ds(struct semid_ds *ds) {
    printf("sem_perm.uid: %d\n", ds->sem_perm.uid);
    printf("sem_perm.gid: %d\n", ds->sem_perm.gid);
    printf("sem_perm.cuid: %d\n", ds->sem_perm.cuid);
    printf("sem_perm.cgid: %d\n", ds->sem_perm.cgid);
    printf("sem_perm.mode: %o\n", ds->sem_perm.mode);
    printf("sem_nsems: %d\n", ds->sem_nsems);
    printf("sem_otime: %s", ctime(&ds->sem_otime));
    printf("sem_ctime: %s \n", ctime(&ds->sem_ctime));
}

int main() {
    int semid;
    union semun arg;
    struct semid_ds ds;

    if (clock_settime(CLOCK_REALTIME, &ts) == -1) { // Set the time to after 2038
        perror("Error setting time");
        return 1;
    }

    // Create a semaphore set
    if ((semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT)) == -1) {
        perror("semget failed");
        exit(1);
    }

    // Get the semid_ds structure
    arg.buf = &ds;
    if (semctl(semid, 0, IPC_STAT, arg) == -1) {
        perror("semctl IPC_STAT failed");
        exit(1);
    }

    // Print the structure contents
    printf("=== semid_ds structure values ===\n");
    print_semid_ds(&ds);


    // Change permissions
    ds.sem_perm.mode = 0600;  // Change to new permissions

    if (semctl(semid, 0, IPC_SET, arg) == -1) {
        perror("semctl IPC_SET failed");
        semctl(semid, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }

    // Print the structure contents
    printf("=== semid_ds structure values ===\n");
    print_semid_ds(&ds);

    if ((ds.sem_ctime - ts.tv_sec > 60) || (ts.tv_sec - ds.sem_ctime > 60)) {
        printf("\nSemctl get a error time! \n");
        exit(EXIT_FAILURE);
    }

    // Delete a semaphore
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID failed");
        exit(1);
    }

    return 0;
}
