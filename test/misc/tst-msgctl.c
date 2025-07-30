#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Define the message structure
struct message {
  long mtype;       // Message Type
  char mtext[100];  // Message body
};

struct timespec ts = {
  .tv_sec = 3468960000,  // 2075-12-05 Destination timestamp
  .tv_nsec = 0
};

void print_msqid_ds(struct msqid_ds *buf) {
  printf("perms: %o\n", buf->msg_perm.mode);
  printf("UID: %d\n", buf->msg_perm.uid);
  printf("GID: %d\n", buf->msg_perm.gid);
  printf("Current number of bytes in the queue:  %d\n", buf->msg_cbytes);
  printf("Number of messages in the queue:  %d\n", buf->msg_qnum);
  printf("Maximum number of bytes allowed in the queue: %d\n", buf->msg_qbytes);
  printf("Last sent time:  %s", buf->msg_stime ? ctime(&buf->msg_stime) + 4 : "Not set \n");
  printf("Last received time:  %s", buf->msg_rtime ? ctime(&buf->msg_rtime) + 4 : "Not set \n");
}


int main() {

  if (clock_settime(CLOCK_REALTIME, &ts) == -1) { // Set the time to after 2038
        perror("Error setting time");
        return 1;
  }

  key_t key = ftok(".", 123);
  if (key == -1) {
    perror("ftok");
    return 1;
  }

  int msqid = msgget(key, 0644 | IPC_CREAT);  // Set to write/read only (not full permissions)
  if (msqid == -1) {
    perror("msgget");
    return 1;
  }

  // Get message queue status
  struct msqid_ds buf;
  memset(&buf, 0, sizeof(buf));  // Clear the structure
  if (msgctl(msqid, IPC_STAT, &buf) == -1) {
    perror("msgctl");
    return 1;
  }

  // Print message queue information
  printf("=== Initial queue status ===\n");
  printf("key: %x\n", key);
  printf("msqid: %d\n", msqid);
  print_msqid_ds(&buf);

  // Prepare the message to be sent
  struct message msg = {0};
  msg.mtype = 1;  // Set the message type to 1
  int i =0;

  for(i =0; i< 2; i++)
  {
  snprintf(msg.mtext, sizeof(msg.mtext), "Hello, Message Queue %d!", i);
  msg.mtext[sizeof(msg.mtext) - 1] = '\0';  // Ensure the message ends with a '\0'

  // Send the message
  if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
    perror("msgsnd");
    return 1;
  }
  printf("Message sent: %s\n", msg.mtext);

  // Check the queue status again
  memset(&buf, 0, sizeof(buf));  // Clear the structure
  if (msgctl(msqid, IPC_STAT, &buf) == -1) {
    perror("msgctl");
    return 1;
  }

  printf("\n=== Queue status after the message is sent ===\n");
  print_msqid_ds(&buf);
  }

  // Change permissions
  buf.msg_perm.mode = 0600;  // New permissions

  if (msgctl(msqid, IPC_SET, &buf) == -1) {
      perror("msgctl IPC_SET failed");
      msgctl(msqid, IPC_RMID, NULL);
      exit(EXIT_FAILURE);
  }

  if ((buf.msg_stime - ts.tv_sec > 60) || (ts.tv_sec - buf.msg_stime > 60)) {
      printf("\nMsgctl get a error time! \n");
      exit(EXIT_FAILURE);
  }

  msgctl(msqid, IPC_RMID, NULL);

  return 0;
}
