#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdatomic.h>

int id; //identifikacijski broj segmenta
//zajednicke varijable 
int *zajednicke; //pravo, zastavica[2], a

void ispis() {
    printf("PRAVO: %d, ZASTAVICE: %d, %d\n\n", *zajednicke, *(zajednicke + 1), *(zajednicke + 2));
}

void uKriticniOdsjecak(int i, int j) {
    *(zajednicke + 1 + i) = 1;
    while(*(zajednicke + 1 + j) != 0) {
        if(*zajednicke == j) {
            *(zajednicke + 1 + i) = 0;
            while(*zajednicke == j);
            *(zajednicke + 1 + i) = 1;
        }
    }
    printf("Proces %d ulazi u K.O.\n", i);
    // ispis();
}

void vanKriticnogOdsjecka(int i, int j) {
    *zajednicke = j;
    *(zajednicke + 1 + i) = 0;
    printf("Proces %d izlazi iz K.O.\n", i);
    // ispis();
}

int brisi(int sig) {
    (void) shmdt((char*) zajednicke);
    (void) shmctl(id, IPC_RMID, NULL);
    exit(0);
}

void proces(int proces, int m) {
    printf("Stvoren proces: %d\n\n", proces);
    for(int i = 0; i < m; i++) {
        uKriticniOdsjecak(proces, (proces + 1)%2);
        *(zajednicke + 3) = *(zajednicke + 3) + 1;
        sleep(1);
        printf("Proces %d, A = %d\n\n", proces, *(zajednicke + 3));
        vanKriticnogOdsjecka(proces, (proces + 1)%2);
        sleep(1);
    }
}

int main() {
    id = shmget(IPC_PRIVATE, sizeof(int) * 4, 0600);
    if(id == -1) exit(1);
    
    zajednicke = (int *) shmat(id, NULL, 0);
    *zajednicke = 0;
    *(zajednicke + 1) = 0;
    *(zajednicke + 2) = 0;
    *(zajednicke + 3) = 0;

    int m;
    printf("Upisite broj M: ");
    scanf("%d", &m);

    for(int i = 0; i < 2; i++) {
        if(fork() == 0) {
            proces(i, m);
            exit(0);
        }
        sleep(1);
    }
        
    for(int i = 0; i < 2; i++) {
        (void) wait(NULL);
    }

    printf("A na kraju = %d\n", *(zajednicke + 3));
    brisi(0);

    return 0;
}