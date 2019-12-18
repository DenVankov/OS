#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <signal.h>

#include "zmq.h"
#include "bank.h"

volatile sig_atomic_t flag = 0;

void block_func(int sig)
{
    if (!flag) {
        flag = 1;
    } else {
        exit(0);
    }
}

void unblock_func(int sig)
{
    flag = 0;
}

int main(void)
{
    int code;
    ClientDB clientBase = ClientDBCreate();
    void *context = zmq_ctx_new();
    void *responsSocket = zmq_socket(context, ZMQ_REP);

    char adress[25];
	printf("This is bank`s server.\n");

    char FileName[STR_SIZE];
    strcpy(FileName, "DataBase");
    printf("Try to load DataBase from %s \n", FileName);

    FILE *file = fopen(FileName, "rb");
    if (file == NULL) {
        printf("Failed to load DataBase.\n");
        printf("Try to create new DataBase.\n");
        printf("DataBase create sucessfully.\n");
    } else {
        ClientDBLoad(clientBase, FileName);
        printf("DataBase loaded sucessfully.\n");
        fclose(file);
    }

    printf("Enter the adress of bank: ");
    ID bank;
    scanf("%d", &bank);
    sprintf(adress, "%s%d", "tcp://*:", bank);

    zmq_bind(responsSocket, adress);

    while (1) {
        signal(SIGINT, block_func);
        signal(SIGTSTP, unblock_func);
        if (!flag) {

            zmq_msg_t message;

            zmq_msg_init(&message);
            zmq_msg_recv(&message, responsSocket, 0);
            MsgData *md = (MsgData *) zmq_msg_data(&message);
            zmq_msg_close(&message);

            char info[STR_SIZE];

            switch (md->action) {
                case 1: {
					printf("Check account ballance ID: %d\n", md->client);
					code = CheckAccount(md->client, clientBase);
					if (code == NOT_CLIENT) {
						printf("Not bank client.\n");
						strcpy(info, "You aren't client of bank.\0");
					} else {
						printf("Client ballance is: %d\n", code);
						ClientDBPrint(clientBase);
						sprintf(info, "%s%d%c", "Your account ballance is: ", code, '\0');
					}

				    memcpy(md->message, info, strlen(info) + 1);
					zmq_msg_init_size(&message, sizeof(MsgData));
					memcpy(zmq_msg_data(&message), md, sizeof(MsgData));
				    zmq_msg_send(&message, responsSocket, 0);
					zmq_msg_close(&message);

					break;
				}

                case 2: {
                    printf("Put money on the account ballance ID: %d\n", md->client);
                    MoneyPut(md->client, md->sum, clientBase);
                    ClientDBPrint(clientBase);
                    strcpy(info, "Operation was completed successfully.\0");

                    memcpy(md->message, info, strlen(info) + 1);
                    zmq_msg_init_size(&message, sizeof(MsgData));
                    memcpy(zmq_msg_data(&message), md, sizeof(MsgData));
                    zmq_msg_send(&message, responsSocket, 0);
                    zmq_msg_close(&message);

                    break;
                }

                case 3: {
                    printf("Get money from the account ballance ID: %d\n", md->client);

                    code = MoneyGet(md->client, md->sum, clientBase);
                    if (code == SUCCESS) {
                        printf("Successfully.\n");
                        ClientDBPrint(clientBase);
                        strcpy(info, "Operation was completed successfully.\0");
                    } else if (code == NOT_ENOUGH_MONEY) {
                        printf("Not enough money.\n");
                        strcpy(info, "You not enough money to withdrawals.\0");
                    } else {
                        printf("Not bank client.\n");
                        strcpy(info, "You aren't a client of bank.\0");
                    }

                    memcpy(md->message, info, strlen(info) + 1);
                    zmq_msg_init_size(&message, sizeof(MsgData));
                    memcpy(zmq_msg_data(&message), md, sizeof(MsgData));
                    zmq_msg_send(&message, responsSocket, 0);
                    zmq_msg_close(&message);

                    break;
                }

                case 4: {
                    printf("Send money from account ID: %d to account ID: %d\n", md->client, md->receiverClient);
                    code = MoneySend(md->client, md->receiverClient, md->sum, clientBase);
                    if (code == SUCCESS) {
                        printf("Successfully.\n");
                        ClientDBPrint(clientBase);
                        strcpy(info, "Operation was completed successfully.\0");
                    } else if (code == NOT_ENOUGH_MONEY) {
                        printf("Not enought money to transfer.\n");
                        strcpy(info, "You not enough money to transfer.");
                    } else if (code == RECEIVER_NOT_CLIENT) {
                        printf("Receiver not bank client\n");
                        strcpy(info, "Receiver account is not a client of bank.\0");
                    }

                    memcpy(md->message, info, strlen(info) + 1);
                    zmq_msg_init_size(&message, sizeof(MsgData));
                    memcpy(zmq_msg_data(&message), md, sizeof(MsgData));
                    zmq_msg_send(&message, responsSocket, 0);
                    zmq_msg_close(&message);
                    break;
                }


            }
            ClientDBSave(clientBase, FileName);
            zmq_msg_close(&message);
        }

    }

    zmq_close(responsSocket);
    zmq_ctx_destroy(context);

    ClientDBDestroy(&clientBase);


}
