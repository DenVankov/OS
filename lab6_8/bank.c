#include "bank.h"

ClientDB ClientDBCreate(void)
{
    ClientDB cDB = (ClientDB) malloc(sizeof(*cDB));
    if (!cDB) {
        fprintf(stderr, "ERROR: no memory\n");
        exit(NOT_MEMORY);
    }
    cDB->clients = (Client) malloc(sizeof(*(cDB->clients)));
    cDB->size = 0;
    cDB->freespace = 1;
    return cDB;
}

void DBResize(ClientDB cDB)
{
    cDB->clients = realloc(cDB->clients, 2 * cDB->size * sizeof(*(cDB->clients)));
    if (!cDB->clients) {
        fprintf(stderr, "ERROR: no memory\n");
        exit(NOT_MEMORY);
    }
    cDB->freespace = cDB->size;
}

void ClientAdd(ClientDB cDB, ID client)
{
    if (!cDB->freespace) {
        DBResize(cDB);
    }

    cDB->clients[cDB->size].client = client;
    cDB->clients[cDB->size].sum = 0;
    cDB->size++;
    cDB->freespace--;
}

void ClientDBPrint(ClientDB cDB)
{
    if (cDB) {
        for (uint32_t i = 0; i < cDB->size; ++i) {
            printf("Client ID: %d\t", cDB->clients[i].client);
            printf("Client ballance: %d\n", cDB->clients[i].sum);
        }
    }
}

Client ClientFind(ClientDB cDB, int clientint)
{
    if (cDB) {
        for (uint32_t i = 0; i < cDB->size; ++i) {
            if (cDB->clients[i].client == clientint) {
                return &(cDB->clients[i]);
            }
        }
    }
    return NULL;
}

void ClientDBDestroy(ClientDB *cDB)
{
    free((*cDB)->clients);
    (*cDB)->clients = NULL;
    free(*cDB);
    *cDB = NULL;
}

/*
void ClientDBSave(ClientDB cDB, char *fileName)
{
    int file = open(fileName, O_RDWR|O_TRUNC|O_CREAT, __S_IREAD|__S_IWRITE);

    if (file < 0) {
        printf("Cannot open file\n");
    } else {
        dup2(file, 1);
        for (int i = 0; i < cDB->size; ++i) {
          printf("%d ",cDB->clients[i].client);
          printf("%d\n",cDB->clients[i].sum);
        }
        //dup2(1,1);
        close(file);
    }

}
*/

void ClientDBSave(ClientDB cDB, char *fileName)
{
    FILE * file = fopen(fileName, "rb");

    if (!file) {
        printf("Cannot open file\n");
    } else {

        for (int i = 0; i < cDB->size; ++i) {
          fprintf(file, "%d ", cDB->clients[i].client);
          fprintf(file, "%d\n", cDB->clients[i].sum);
        }
        fclose(file);
    }

}

/*void ClientDBLoad(ClientDB cDB, char *fileName)
{
    FILE * file = fopen(fileName, "rb");

    if (!file) {
        printf("Cannot open file\n");
    } else {
        int id = 0;
        int balance = 0;
        printf("Start scaning data...\n");

        int i = 0;
        while(fscanf(file, "%d%d", &id, &balance) != EOF) {
            cDB->clients[i].client = id;
            cDB->clients[i].sum = balance;
            printf("The id: %d ", cDB->clients[i].client);
            printf("The balance: %d\n", cDB->clients[i].sum);
            printf("Scanned %d\n", i);
            MoneyPut(cDB->clients[i].client, cDB->clients[i].sum, cDB);
            ++i;
        }
        printf("Data succesfully scaned.");
    }
    fclose(file);
}
*/

void ClientDBLoad(ClientDB cDB, char *fileName)
{
    int file = open(fileName, O_RDONLY|O_CREAT, __S_IREAD|__S_IWRITE);

    if (!file) {
        printf("Cannot open file\n");
    } else {
        int id = 0;
        int balance = 0;
        printf("Start scaning data...\n");

        int i = 0;
        char symb;
        while(read(file, &symb, sizeof(char))) {

            while(symb != ' ') {
                id = id * 10 + (symb - '0');
                read(file, &symb, sizeof(char));
            }
            printf("The id: %d ", id);
            cDB->clients[i].client = id;

            read(file, &symb, sizeof(char));
            while(symb != '\n') {
                balance = balance * 10 + (symb - '0');
                read(file, &symb, sizeof(char));
            }
            printf("The balance: %d\n", balance);
            cDB->clients[i].sum = balance;

            MoneyPut(cDB->clients[i].client, cDB->clients[i].sum, cDB);
            id = 0;
            balance = 0;
            ++i;
      }
        printf("Data succesfully scaned.\n");
        close(file);
    }

}

void ClientAccIncrease(Client cl, int sum)
{
    cl->sum += sum;
}

void ClientAccDecrease(Client cl, int sum)
{
    cl->sum -= sum;
}

int ClientHasEnoughMoney(Client cl, int sum)
{
    return cl->sum >= sum;
}

void MoneyPut(int clientint, int sum, ClientDB cDB)
{
    Client cl = ClientFind(cDB, clientint);

    if (cl) {
        ClientAccIncrease(cl, sum);
    } else {
        ClientAdd(cDB, clientint);
        cl = ClientFind(cDB, clientint);
        ClientAccIncrease(cl, sum);
    }
}

int MoneyGet(int clientint, int sum, ClientDB cDB)
{
    Client cl = ClientFind(cDB, clientint);
    if (!cl) {
        return NOT_CLIENT;
    }
    if (ClientHasEnoughMoney(cl, sum)) {
        ClientAccDecrease(cl, sum);
        return SUCCESS;
    } else {
        return NOT_ENOUGH_MONEY;
    }
}

int MoneySend(ID clientSender, ID clientReceiver, int sum, ClientDB cDB)
{
    Client clSender = ClientFind(cDB, clientSender);
    if (!clSender) {
        return NOT_CLIENT;
    }
    Client clReceiver = ClientFind(cDB, clientReceiver);
    if (!clReceiver) {
        return RECEIVER_NOT_CLIENT;
    }

    if (ClientHasEnoughMoney(clSender, sum)) {
        ClientAccDecrease(clSender, sum);
        ClientAccIncrease(clReceiver, sum);
        return SUCCESS;
    } else {
        return NOT_ENOUGH_MONEY;
    }
}

int CheckAccount(ID client, ClientDB cDB)
{
    Client cl = ClientFind(cDB, client);
    if (!cl) {
        return NOT_CLIENT;
    }
    return cl->sum;
}
