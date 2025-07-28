#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

// Define the message structure
struct msg_buffer {
    long msg_type;
    char msg_text[100];
};

void print_queue_times(int msgid) {
     struct msqid_ds info;
     if (msgctl(msgid, IPC_STAT, &info) == -1) {
	 perror("msgctl IPC_STAT failed");
         exit(1);
     }

     printf("Last send time (msg_stime): %s", ctime(&info.msg_stime));     
     printf("Last recv time (msg_rtime): %s", ctime(&info.msg_rtime));     
     printf("Last change time (msg_ctime): %s", ctime(&info.msg_ctime));     

}

int main() {
    key_t key;
    int msgid;
    struct msqid_ds buf;

    // Generate a unique key
    key = ftok("progfile", 65);
   
    // Create a message queue and return its id
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    // Prepare message to send
    struct msg_buffer message;
    message.msg_type = 1;
    strcpy(message.msg_text, "Hello from message queue!");

    // Send the message
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) < 0) {
	perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    printf("Message sent: %s\n", message.msg_text);
    print_queue_times(msgid);  // Show msg_stime after sending

    // Get information about the message queue
    if (msgctl(msgid, IPC_STAT, &buf) == -1) {
        perror("msgctl - IPC_STAT failed");
        exit(EXIT_FAILURE);
    }

    // Print some info from the msqid_ds structure
    printf("Message queue ID: %d\n", msgid);

    // Receive the message
    struct msg_buffer received;
    if (msgrcv(msgid, &received, sizeof(received.msg_text), 1, 0) < 0) {
	perror("msgrcv failed");
        exit(EXIT_FAILURE);
    }
    printf("Message received: %s\n", received.msg_text);
    print_queue_times(msgid);  // Show msg_stime after receiving

    // Remove the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl - IPC_RMID failed");
        exit(EXIT_FAILURE);
    }

    printf("Message queue removed successfully.\n");

    return 0;
}
