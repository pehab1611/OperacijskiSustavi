#include <stdio.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int id; 
typedef struct {
    int otvoreno;
    int znak_otvoreno;
    int br_klijenata;
    int tren_klijent;
} zajednicke;

zajednicke *data;
sem_t *otv;
sem_t *kli;
sem_t *frizurka;
sem_t *sjedi;
sem_t *radim;

void frizerka() {
    sem_wait(otv);
    data->otvoreno = 1;
    sem_post(otv);
    printf("FRIZERKA: Otvoram salon\n");
    printf("FRIZERKA: Postavljam znak OTVORENO\n");

    int ispisano = 0; 
    int zatv = 0;
    while(1) {
        sem_wait(otv);
        if(data->otvoreno == 0) {
            if(zatv == 0) printf("FRIZERKA: Postavljam znak ZATVORENO\n");
            data->znak_otvoreno = 1;
            zatv = 1;
        }
        sem_post(otv);
        sem_wait(kli);
        if(data->br_klijenata > 0) {
            sem_post(otv);
            sem_post(kli);
            sem_post(frizurka);
            sem_wait(sjedi);
            printf("FRIZERKA: Idem raditi na klijentu %d\n", data->tren_klijent);
            sem_post(radim);
            sleep(4);
            printf("FRIZERKA: Klijent %d gotov\n", data->tren_klijent);
            ispisano = 0;
        } else if(data->znak_otvoreno == 0) {
            if(ispisano == 0) printf("FRIZERKA: Spavam dok klijenti ne dođu\n");
            ispisano = 1;
            sem_post(otv);
            sem_post(kli);
        } else {
            printf("FRIZERKA: Zatvaram salon\n");
            sem_post(otv);
            sem_post(kli);
            return;
        }
    }
}

void klijent(int i) {
    printf("\t\tKLIJENT(%d): Želim na frizuru\n", i);
    sem_wait(otv);
    if(data->znak_otvoreno == 1) {
        printf("\t\tKLIJENT(%d): Zatvoreno, vraćam se sutra\n", i);
        sem_post(otv);
        sem_wait(kli);
        return;
    }
    sem_post(otv);
    sem_wait(kli);
    if(data->br_klijenata == 3) {
        printf("\t\tKLIJENT(%d): Nema mjesta, vraćam se sutra\n", i);
        sem_post(kli);
    } else {
        data->br_klijenata++;
        printf("\t\tKLIJENT(%d): Ulazim u čekaonu(%d)\n", i, data->br_klijenata);
        sem_post(kli);
        sem_wait(frizurka);
        sem_wait(kli);
        data->br_klijenata--;
        sem_post(kli);
        data->tren_klijent = i;
        sem_post(sjedi);
        sem_wait(radim);
        //-----------------------------------------
        printf("\t\tKLIJENT(%d): Frizerka mi radi frizuru\n", i); //WHY DOESN'T THIS LINE PRINT?
    }
    return;
}

void stvori_klijente() {
    int i = 1;
    while(i <= 10) {
        int rand_num;
        srand(time(NULL));
        rand_num = (rand() % 2) + 1;
        sleep(rand_num);
        if(fork() == 0) {
            klijent(i);
            return;
        }
        i++;
    }
}

int main() {
    id = shmget(IPC_PRIVATE, sizeof(zajednicke) + 6 * sizeof(sem_t), 0600);
    if(id == -1) exit(1);

    data = (zajednicke *) shmat(id, NULL, 0);

    otv = (sem_t *)((char*)data + sizeof(zajednicke));
    kli = (sem_t *)((char*)otv + sizeof(sem_t));
    frizurka = (sem_t *)((char*)kli + sizeof(sem_t));
    sjedi = (sem_t *)((char*)frizurka + sizeof(sem_t));
    radim = (sem_t *)((char*)sjedi + sizeof(sem_t));

    data->otvoreno = 0;
    data->br_klijenata = 0;
    data->tren_klijent = 0;

    sem_init(otv, 1, 1);
    sem_init(kli, 1, 1);
    sem_init(frizurka, 1, 0);
    sem_init(sjedi, 1, 0);
    sem_init(radim, 1, 0);

    if(fork() == 0) {
        frizerka();
        shmdt(data);
        return 0;
    } else {
        sleep(1);
        if(fork() == 0) {
            stvori_klijente();
            return 0;
        }
    }

    sleep(30);
    sem_wait(otv);
    data->otvoreno = 0;
    sem_post(otv);

    for(int i = 0; i < 10 + 1; i++) {
        (void) wait(NULL);
    }

    sem_destroy(otv);
    sem_destroy(kli);
    sem_destroy(frizurka);
    sem_destroy(sjedi);
    sem_destroy(radim);

    (void) shmdt((char*) (sizeof(zajednicke) + 6 * sizeof(sem_t)));
    (void) shmctl(id, IPC_RMID, NULL);

    return 0;
}