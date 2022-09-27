#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

struct msgstr{
    long type;
    char msg[1];
};

int main()
{
    key_t key1 = ftok("/home", 'a');
    int shmd;
    while((shmd = shmget(key1, sizeof(int) * 9, 0666)) < 0);
    int* addr = shmat(shmd, NULL, 0);

    key_t key2 = ftok("/home", 'b');
    int msgd;
    while((msgd = msgget(key2, 0666)) < 0);

    struct msgstr s;
    msgrcv(msgd, &s, 2, 2, 0);
    printf("Player 1 is Here!\n");
    s.type = 1;
    s.msg[0] = '2';
    msgsnd(msgd, &s, 1, 0);
    int l, c;
    do{
        msgrcv(msgd, &s, 2, 2, 0);
        if(s.msg[0] == '0')
            break;
        printf("\nInput a line and a column of CROSS:\n");
        scanf("%d %d\n", &l, &c);
        while(l > 3 || l < 1 || c > 3 || c < 1 || addr[(l - 1)*3 + c - 1] != 0){
            printf("Wrong coordinates, try again!\n");
            scanf("%d %d", &l, &c);
        }
        addr[(l - 1)*3 + c - 1] = 1;
        s.type = 1;
        s.msg[0] = '2';
        msgsnd(msgd, &s, 1, 0);
    } while(1);
    return 0;
}
