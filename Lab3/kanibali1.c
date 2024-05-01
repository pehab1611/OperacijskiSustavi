#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

char type_desna_obala[100], type_lijeva_obala[100], type_u_camcu[7];
int  br_desna_obala[100], br_lijeva_obala[100], br_u_camcu[7];
int len_desna = 0, len_lijeva = 0, len_camac = 0;
int tren_obala = 0; //D = 0; L = 1
int br_kanibala_na_brodu = 0, br_misionara_na_brodu = 0; 

pthread_mutex_t lObala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dObala = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cam = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lOb = PTHREAD_COND_INITIALIZER;
pthread_cond_t dOb = PTHREAD_COND_INITIALIZER;
pthread_cond_t tri = PTHREAD_COND_INITIALIZER;

char rand_obala() {
    return rand() % 2;
}

void ispis() {
    int i = 0;
    if (tren_obala == 0) {
        printf("C[D]={ ");
    } else {
        printf("C[L]={ ");
    }
    while(i < len_camac) {
        printf("%c%d ", type_u_camcu[i], br_u_camcu[i++]);
    }
    printf("} LO={ ");
    i = 0;
    while(i < len_lijeva) {
        if(br_lijeva_obala[i] > 0) printf("%c%d ", type_lijeva_obala[i], br_lijeva_obala[i++]);
        else i++;
    }
    printf("} DO={ ");
    i = 0;
    while(i < len_desna) {
        if(br_desna_obala[i] > 0) printf("%c%d ", type_desna_obala[i], br_desna_obala[i++]);
        else i++;
    }
    printf("}\n\n");
}

void *misionar(void *x) {
    int broj = *((int*)x);
    int strana = rand_obala();
    int moja_pozicija;
    if(strana == 0) {
        //Desna strana
        pthread_mutex_lock(&dObala);
        printf("M%d: došao na desnu stranu\n", broj);
        moja_pozicija = len_desna;
        type_desna_obala[len_desna] = 'M';
        br_desna_obala[len_desna++] = broj;
        ispis();
        pthread_mutex_unlock(&dObala);

        pthread_mutex_lock(&cam);
        while(tren_obala != strana || len_camac >= 7 || br_misionara_na_brodu + 1 < br_kanibala_na_brodu) {
            pthread_cond_wait(&dOb, &cam);
        }

        pthread_mutex_lock(&dObala);
        br_desna_obala[moja_pozicija] = 0;
        pthread_mutex_unlock(&dObala);

        type_u_camcu[len_camac] = 'M';
        br_u_camcu[len_camac++] = broj;
        br_misionara_na_brodu++;
        if(len_camac == 3) {
            pthread_cond_signal(&tri);
        }
        printf("M%d: ušao u čamac\n", broj);
        ispis();
        pthread_cond_broadcast(&dOb);
        pthread_mutex_unlock(&cam);
    } else {
        //Lijeva strana
        pthread_mutex_lock(&lObala);
        printf("M%d: došao na lijevu stranu\n", broj);
        moja_pozicija = len_lijeva;
        type_lijeva_obala[len_lijeva] = 'M';
        br_lijeva_obala[len_lijeva++] = broj;
        ispis();
        pthread_mutex_unlock(&lObala);

        pthread_mutex_lock(&cam);
        while(tren_obala != strana || len_camac >= 7 || br_misionara_na_brodu + 1 < br_kanibala_na_brodu) {
            pthread_cond_wait(&lOb, &cam);
        }

        pthread_mutex_lock(&lObala);
        br_lijeva_obala[moja_pozicija] = 0;
        pthread_mutex_unlock(&lObala);

        type_u_camcu[len_camac] = 'M';
        br_u_camcu[len_camac++] = broj;
        br_misionara_na_brodu++;
        if(len_camac == 3) {
            pthread_cond_signal(&tri);
        }
        printf("M%d: ušao u čamac\n", broj);
        ispis();
        pthread_cond_broadcast(&lOb);
        pthread_mutex_unlock(&cam);
    }
}

void *kanibal(void *x) {
    int broj = *((int*)x);
    broj = broj - 2;
    int strana = rand_obala();
    int moja_pozicija; 
    if(strana == 0) {
        //Desna strana
        pthread_mutex_lock(&dObala);
        printf("K%d: došao na desnu stranu\n", broj);
        moja_pozicija = len_desna;
        type_desna_obala[len_desna] = 'K';
        br_desna_obala[len_desna++] = broj;
        ispis();
        pthread_mutex_unlock(&dObala);

        pthread_mutex_lock(&cam);
        while(tren_obala != strana || len_camac >= 7 || (br_kanibala_na_brodu + 1 > br_misionara_na_brodu && br_misionara_na_brodu > 0)) {
            pthread_cond_wait(&dOb, &cam);
        }

        pthread_mutex_lock(&dObala);
        br_desna_obala[moja_pozicija] = 0;
        pthread_mutex_unlock(&dObala);

        type_u_camcu[len_camac] = 'K';
        br_u_camcu[len_camac++] = broj;
        br_kanibala_na_brodu++;
        if(len_camac == 3) {
            pthread_cond_signal(&tri);
        }
        printf("K%d: ušao u čamac\n", broj);
        ispis();
        pthread_cond_broadcast(&dOb);
        pthread_mutex_unlock(&cam);
    } else {
        //Lijeva strana
        pthread_mutex_lock(&lObala);
        printf("K%d: došao na lijevu stranu\n", broj);
        moja_pozicija = len_lijeva;
        type_lijeva_obala[len_lijeva] = 'K';
        br_lijeva_obala[len_lijeva++] = broj;
        ispis();
        pthread_mutex_unlock(&lObala);

        pthread_mutex_lock(&cam);
        while(tren_obala != strana || len_camac >= 7 || (br_kanibala_na_brodu + 1 > br_misionara_na_brodu && br_misionara_na_brodu > 0)) {
            pthread_cond_wait(&lOb, &cam);
        }

        pthread_mutex_lock(&lObala);
        br_lijeva_obala[moja_pozicija] = 0;
        pthread_mutex_unlock(&lObala);

        type_u_camcu[len_camac] = 'K';
        br_u_camcu[len_camac++] = broj;
        br_kanibala_na_brodu++;
        if(len_camac == 3) {
            pthread_cond_signal(&tri);
        }
        printf("K%d: ušao u čamac\n", broj);
        ispis();
        pthread_cond_broadcast(&lOb);
        pthread_mutex_unlock(&cam);
    }
}

void *camac(void *x) {
    if(tren_obala == 0) printf("C: prazan na desnoj obali\n");
    else printf("C: prazan na lijevoj obali\n");
    ispis();
    while(1) {
        pthread_mutex_lock(&cam);
        while(len_camac < 3) {
            pthread_cond_wait(&tri, &cam);
        }
        printf("C: tri putnika ukrcana, polazim za jednu sekundu\n");
        ispis();
        pthread_mutex_unlock(&cam);

        sleep(1);

        pthread_mutex_lock(&cam);
        if(tren_obala == 0) printf("C: prevozim s desne na lijevu obalu: ");
        else printf("C: prevozim s lijeve na desnu obalu: ");
        int i = 0;
        while(i < len_camac) {
            printf("%c%d ", type_u_camcu[i], br_u_camcu[i++]);
        }
        printf("\n");

        sleep(2);

        if(tren_obala == 0) printf("C: preveo s desne na lijevu obalu: ");
        else printf("C: preveo s lijeve na desnu obalu: ");
        i = 0;
        while(i < len_camac) {
            printf("%c%d ", type_u_camcu[i], br_u_camcu[i++]);
        }
        printf("\n");        
        len_camac = 0;
        br_kanibala_na_brodu = 0;
        br_misionara_na_brodu = 0;
        tren_obala = (tren_obala + 1) % 2;
        if(tren_obala == 0) {
            printf("C: prazan na desnoj obali\n");
            pthread_cond_broadcast(&dOb);
            }
        else {
            printf("C: prazan na lijevoj obali\n");
            pthread_cond_broadcast(&lOb);
        }
        ispis();
        pthread_mutex_unlock(&cam);
    }
}

void *stvaraj_misionare(void *x) {
    pthread_t thr_id[50];
    for(int i = 0; i <= 50; i++) {
        sleep(2);
        if(pthread_create(&thr_id[i], NULL, misionar, &i) != 0) {
            printf("Greska pri stvaranju %d misionar dretve\n", i);
            exit(1);
        }
    }

    for(int i = 0; i < 50; i++) {
        pthread_join(thr_id[i], NULL);
    }
}

int main() {
    srand(time(NULL));
    pthread_t thr_id[52];
    
    pthread_mutex_init(&dObala, NULL);
    pthread_mutex_init(&lObala, NULL);
    pthread_cond_init(&lOb, NULL);
    pthread_cond_init(&dOb, NULL);
    pthread_cond_init(&tri, NULL);

    if(pthread_create(&thr_id[0], NULL, camac, NULL) != 0) {
        printf("Greska pri stvaranju 'camac' dretve\n");
        exit(1);
    }
    if(pthread_create(&thr_id[1], NULL, stvaraj_misionare, NULL) != 0) {
        printf("Greska pri stvaranju 'stvaraj_misionare' dretve\n");
        exit(1);
    }
    for(int i = 2; i <= 52; i++) {
        sleep(1);
        if(pthread_create(&thr_id[i], NULL, kanibal, &i) != 0) {
            printf("Greska pri stvaranju %d kanibal dretve\n", i);
            exit(1);
        }
    }

    for(int i = 0; i < 52; i++) {
        pthread_join(thr_id[i], NULL);
    }
}