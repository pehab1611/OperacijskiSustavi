#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>

int A; 
int *ulaz;
int *broj;

void uKriticniOdsjecak(int i, int n) {
    ulaz[i] = 1;
    int max = 0;
    for(int j = 0; j < n; j++) {
        if(broj[j] > max) max = broj[j];
    }
    broj[i] = max + 1;
    ulaz[i] = 0;

    for(int j = 0; j < n; j++) {
        while(ulaz[j] != 0);
        while(broj[j] != 0 && (broj[j] < broj[i] || (broj[j] == broj[i] && j < i)));
    } 
    printf("Proces %d ušao u K.O.\n", i);
}

void vanKriticnogOdsjecka(int i) {
    broj[i] = 0;
    printf("Proces %d izlazi iz K.O.\n\n", i);
}

void *dretva(void *x) {
    int m = *((int*)x);
    int n = *((int*)x + 1);
    int i = *((int*)x + 2);

    for(int j = 0; j < m; j++) {
        uKriticniOdsjecak(i, n);
        A++;
        sleep(1);
        vanKriticnogOdsjecka(i);
    }
}

int main(void) {
    int n, m;
    printf("Upisite N: ");
    scanf("%d", &n);
    printf("Upisite M: ");
    scanf("%d", &m);

    A = 0; 
    pthread_t thr_id[n];
    ulaz = calloc(n, sizeof(int));
    broj = calloc(n, sizeof(int));

    int brojevi[] = {m, n, 0};
    for(int i = 0; i < n; i++) {
        brojevi[2] = i; 
        if(pthread_create(&thr_id[i], NULL, dretva, &brojevi) != 0) {
            printf("Greska pri stvaranju %d. dretve\n", i);
            exit(1);
        }
        printf("Stvorena %d dretva\n\n", i);
        sleep(1);
    }

    for(int i = 0; i < n; i++) {
        pthread_join(thr_id[i], NULL);
    }

    printf("A na kraju: %d\n", A);

    free(ulaz);
    free(broj);
    return 0; 
}