#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

void out_square(int* a);
int is_fin(int* a);

struct msgstr{
    long type;
    char msg[1];
};

int main()
{
    key_t key1 = ftok("/home", 'a');
    int shmd = shmget(key1, sizeof(int) * 9, 0666 | IPC_CREAT | IPC_EXCL);
    if(shmd < 0){
        printf("Cannot create a sharing memory\n");
        return 0;
    }
    int* addr = shmat(shmd, NULL, 0);
    key_t key2 = ftok("/home", 'b');
    int msgd = msgget(key2, 0666 | IPC_CREAT | IPC_EXCL);
    if(msgd < 0){
        printf("Cannot create a message line\n");
        shmctl(shmd, IPC_RMID, NULL);
        return 0;
    }
    struct msgstr s;
    for(int i = 0; i < 9; i++){
        addr[i] = 0;
    }
    printf("\n");
    s.type = 2;
    s.msg[0] = '1';
    msgsnd(msgd, &s, 1, 0);
    msgrcv(msgd, &s, 2, 1, 0);
    s.type = 3;
    s.msg[0] = '1';
    msgsnd(msgd, &s, 1, 0);
    msgrcv(msgd, &s, 2, 1, 0);
    printf("\nOkey, that mean, you may play tick-tack-toe!\n");
    printf("Here is the square:\n");
    out_square(addr);
    do{
        s.type = 2;
        s.msg[0] = '1';
        msgsnd(msgd, &s, 1, 0);
        msgrcv(msgd, &s, 2, 1, 0);
        printf("\033[2J");
        printf("\033[0;0f");
        out_square(addr);
        if(is_fin(addr) != 0)
            break;
        s.type = 3;
        s.msg[0] = '1';
        msgsnd(msgd, &s, 1, 0);
        msgrcv(msgd, &s, 2, 1, 0);
        printf("\033[2J");
        printf("\033[0;0f");
        out_square(addr);
    } while(is_fin(addr) == 0);
    s.type = 2;
    s.msg[0] = '0';
    msgsnd(msgd, &s, 1, 0);
    s.type = 3;
    s.msg[0] = '0';
    msgsnd(msgd, &s, 1, 0);
    shmctl(shmd, IPC_RMID, NULL);
    msgctl(msgd, IPC_RMID, NULL);
    return 0;
}

int is_fin(int* a)
{
    int fin = 0;
    for(int i = 0; i < 9; i+=3){
        if(a[i] == a[i + 1] && a[i] == a[i + 2] && a[i] != 0){
            fin = a[i];
            break;
        }
    }
    if(fin == 0)
        for(int i = 0; i < 3; i++){
        if(a[i] == a[i + 3] && a[i] == a[i + 6] && a[i] != 0){
            fin = a[i];
            break;
        }
    }
    if(fin == 0){
        if(a[0] == a[4] && a[0] == a[8] && a[0] != 0)
            fin = a[0];
        if(a[2] == a[4] && a[2] == a[6] && a[2] != 0)
            fin = a[2];
    }
    if(fin == 0){
        int i;
        for(i = 0; i < 9; i++){
            if(a[i] == 0)
                break;
        }
        if(i == 9)
            fin = 3;
    }
    if(fin == 0)
        return 0;
    if(fin == 1)
        printf("\n CROSS won!\n");
    else if(fin == 2)
        printf("\n TOE won!\n");
    else
        printf("\n Nobody won!\n");
    return 1;
}

void out_square(int* a)
{
    int k;
    printf("\n      1 : 2 : 3\n");
    printf("    +---+---+---+\n");
    for(int i = 0; i < 3; i++){
        printf("  %d :", i + 1);
        for(int j = 0; j < 3; j++){
            k = a[3*i + j];
            if(k == 0)
                printf("   :");
            else if(k == 1)
                printf(" X :");
            else if(k == 2)
                printf(" O :");
            else{
                printf("\nSorry, something went wrong\n");
                exit(0);
            }
        }
        printf("\n  --+---+---+---+\n");
    }
}
