#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <pthread.h>

#include "zmq.h"
#include "bank.h"

void menuAdmin()
{
    printf(">> Press 1) Stop server \n");
    printf(">> Press 2) Run server \n");
    printf(">> Press 3) Exit \n");
}

void menuUser()
{
    printf("_______________________________________________\n");
    printf(">> Press 1) Check balance\n");
    printf(">> Press 2) Put money on your account balance\n");
    printf(">> Press 3) Get money from your account balance\n");
    printf(">> Press 4) Send money to another account\n");
    printf(">> Press 5) Get help\n");
    printf(">> Press 6) Exit the bank\n");
    printf("_______________________________________________\n");

}

void *SendRecv(void *arg)
{
    MsgData *md = (MsgData *) arg;
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(MsgData));
    memcpy(zmq_msg_data(&message), md, sizeof(MsgData));
    zmq_msg_send(&message, md->requester, 0);
    zmq_msg_close(&message);

    zmq_msg_init(&message);
    zmq_msg_recv(&message, md->requester, 0);
    md = (MsgData *) zmq_msg_data(&message);
    printf("%s\n", md->message);
    zmq_msg_close(&message);
    pthread_exit(NULL);
    return 0;
}

int main(int argc, char **argv)
{
    void *context = zmq_ctx_new();
    int admin = 0;

    ID client, bank;
    if (argc == 2 && !strcmp(argv[1], "admin")) {
        admin = 1;
    } else {
        printf("Registration...\n");
        printf("Enter client's login: ");
        scanf("%d", &client);
    }

    char adress[25];
    printf("Enter bank's adress: ");
    scanf("%d", &bank);

    sprintf(adress, "%s%d", "tcp://localhost:", bank);

    printf("tcp://localhost:%d \n", bank);

    void *sendSocket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(sendSocket, adress);

    if (admin) {
        int act = 0;
        menuAdmin();
        do {
            scanf("%d", &act);
            MsgData md;
            md.action = act + 10;
            switch (act) {

                case 1: {
                    pthread_t th;
                    md.requester = sendSocket;
                    pthread_create(&th, NULL, SendRecv, &md);
                    pthread_detach(th);
                    break;
                }

                case 2: {
                    pthread_t th;
                    md.requester = sendSocket;
                    pthread_create(&th, NULL, SendRecv, &md);
                    pthread_detach(th);
                    break;
                }

                case 3:
                    break;

                default: {
                    printf("Inccorect command\n");
                    break;
                }
            }
        } while (act != 3);


    } else {
        int act = 0, sum = 0;
        menuUser();
        do {
            scanf("%d", &act);

            MsgData md;
            md.action = act;
            md.client = client;

            switch (act) {

                case 1: {
                  pthread_t th;
                  md.requester = sendSocket;
                  pthread_create(&th, NULL, SendRecv, &md);
                  pthread_detach(th);

                  break;
                }

                case 2: {
                    printf("Enter a number of money to deposit: ");
                    scanf("%d", &sum);
                    while (sum < 0) {
                      printf("Inccorect number of money. Plese try again: ");
                      scanf("%d", &sum);
                    }
                    md.sum = sum;
                    pthread_t th;
                    md.requester = sendSocket;
                    pthread_create(&th, NULL, SendRecv, &md);
                    pthread_detach(th);
                    break;
                }

                case 3: {
                    printf("Enter a number of money to withdrawals: ");
                    scanf("%d", &sum);
                    while (sum < 0) {
                      printf("Inccorect number of money. Plese try again: ");
                      scanf("%d", &sum);
                    }
                    md.sum = sum;
                    pthread_t th;
                    md.requester = sendSocket;
                    pthread_create(&th, NULL, SendRecv, &md);
                    pthread_detach(th);

                    break;
                }

                case 4: {
                    int receiverClient;
                    printf("Enter receiver`s client ID: ");
                    scanf("%d", &receiverClient);

                    printf("Enter a number of money to transfer: ");
                    scanf("%d", &sum);
                    while (sum < 0) {
                      printf("Inccorect number of money. Plese try again: ");
                      scanf("%d", &sum);
                    }
                    md.sum = sum;
                    md.receiverClient = receiverClient;

                    pthread_t th;
                    md.requester = sendSocket;
                    pthread_create(&th, NULL, SendRecv, &md);
                    pthread_detach(th);

                    break;
                }

                case 5: {
                  menuUser();
                  break;
                }

                case 6:
                    break;

                default: {
                    printf("Command not found.\n");
                    break;
                }
            }
        } while (act != 6);
    }

    zmq_close(sendSocket);
    zmq_ctx_destroy(context);

    return 0;
}
