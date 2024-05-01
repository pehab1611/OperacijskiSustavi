#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int t = 0;
int m, n;
int *tablica_prevodjenja, *okvir, *disk;
int vel_tablica = 16;
int vel_okvir = 64, vel_disk = 64;

int popunjenost = 0;

void dohvati_fizicku_adresu(int proces, int log_adresa) {
    int pomak_okvir = log_adresa & 0b111111;
    int index_stranicenje = (log_adresa >> 6);

    // printf("%d %d\n", pomak_okvir, index_stranicenje);
    // printf("%d\n", proces * vel_tablica + index_stranicenje);

    int zapis = *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje);
    int ok = zapis >> 6;
    int p = (zapis >> 5) & 0b1;
    int lru = zapis & 0b11111;
    // printf("ZAPIS: %d %d %d %d\n", zapis, ok, p, lru);

    if(!((zapis >> 5) & 0x1)) {
        printf("\tPROMAŠAJ!\n");
        if(popunjenost < m) {
            *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje) = 0b100000 + t + popunjenost * 64;
            printf("\t\tdodijeljen okvir 0x%04x\n", popunjenost);
            for(int i = 0; i < vel_okvir; i++) {
                *(okvir + popunjenost * vel_okvir + i) = *(disk + proces * vel_disk + i);
            }

            // printf("OKVIR: \n");
            // for(int i = 0; i < m; i++) {
            //     for(int j = 0; j < vel_okvir; j++) {
            //         printf("%d ", *(okvir + i * vel_okvir + j));
            //     }
            //     printf("\n");
            // }
            // printf("\n");
            popunjenost++;
        } else {
            int min_lru = 32;
            int min_i, min_j, min_adresa;
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < vel_tablica; j++) {
                    int lru1 = *(tablica_prevodjenja + i * vel_tablica + j) & 0b11111;
                    // printf("LRU: %d %x\n", *(tablica_prevodjenja + i * vel_tablica + j), lru1);
                    if(lru1 < min_lru && (*(tablica_prevodjenja + i * vel_tablica + j) >> 5) & 0b1 == 1) {
                        // printf("Promjena\n");
                        min_lru = lru1;
                        min_i = i;
                        min_j = j;
                        min_adresa = *(tablica_prevodjenja + i * vel_tablica + j) >> 6;
                    }
                }
            }

            // printf("Okvir %d zapisujem na disk %d\n Disk %d zapisuje na okvir %d\n", min_adresa, min_i, min_adresa, proces);
            for (int i = 0; i < vel_disk; i++) {
                *(disk + min_i * vel_disk + i) = *(okvir + min_adresa * vel_okvir + i);
                *(okvir + min_adresa * vel_okvir + i) = *(disk + proces * vel_disk + i);   
            }

            // printf("OKVIR: \n");
            // for(int i = 0; i < m; i++) {
            //     for(int j = 0; j < vel_okvir; j++) {
            //         printf("%d ", *(okvir + i * vel_okvir + j));
            //     }
            //     printf("\n");
            // }

            // printf("\nDISK: \n");
            // for(int i = 0; i < n; i++) {
            //     for(int j = 0; j < vel_disk; j++) {
            //         printf("%d ", *(disk + i * vel_disk + j));
            //     }
            //     printf("\n");
            // }

            printf("\t\tIzbacujem stranicu 0x%04x iz procesa %d\n", min_j << 6, min_i);
            printf("\t\tlru izbacene stranice: 0x%04x\n", min_lru);
            printf("\t\tdodijeljen okvir: 0x%04x\n", min_adresa);

            *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje) = 0x20 + t + min_adresa * 64;
            *(tablica_prevodjenja + min_i * vel_tablica + min_j) = *(tablica_prevodjenja + min_i * vel_tablica + min_j) & 0xffdf;
        } 
    } else {
        printf("\t\tprisutan u okviru\n");
        *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje) = *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje) - lru + t;
        // printf("OKVIR: \n");
        //     for(int i = 0; i < m; i++) {
        //         for(int j = 0; j < vel_okvir; j++) {
        //             printf("%d ", *(okvir + i * vel_okvir + j));
        //         }
        //     printf("\n");
        // }
    }
    int fiz_adresa = ok + pomak_okvir;
    zapis = *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje);
    printf("\tfiz. adresa: 0x%04x\n", fiz_adresa);
    printf("\tzapis tablice: 0x%04x\n", zapis);
    printf("\tsadrzaj adrese: %d\n", *(okvir + (zapis >> 6) * vel_okvir + pomak_okvir));

    (*(okvir + (zapis >> 6) * vel_okvir + pomak_okvir))++;

    if(t == 31) {
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < vel_tablica; j++) {
                *(tablica_prevodjenja + i * vel_tablica + j) = *(tablica_prevodjenja + i * vel_tablica + j) - (*(tablica_prevodjenja + i * vel_tablica + j) & 0b11111);
            }
        }
        *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje) = *(tablica_prevodjenja + proces * vel_tablica + index_stranicenje) + 1;
        t = 1;
    }
}

int main() {
    srand(time(0));

    printf("Upišite broj okvira M:");
    scanf("%d", &m);
    printf("Upišite broj procesa N:");
    scanf("%d", &n);

    tablica_prevodjenja = malloc(sizeof(int) * n * vel_tablica);
    okvir = malloc(sizeof(int) * m * vel_okvir);
    disk = malloc(sizeof(int) * n * vel_disk);

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < vel_disk; j++) {
            *(disk + i * vel_disk + j) = 0;
        }
    }

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < vel_tablica; j++) {
            *(tablica_prevodjenja + i * vel_tablica + j) = 0;
        }
    }

    for(int i = 0; i < m; i++) {
        for(int j = 0; j < vel_okvir; j++) {
            *(okvir + i * vel_okvir + j) = 0;
        }
    }

    t = 0;
    int log_adresa;

    while(1) {
        for(int i = 0; i < n; i++) {
            // log_adresa = rand() & 0x3FE;
            log_adresa = 0x100;

            printf("---------------------------\n");
            printf("proces: %d\n", i);
            printf("\tt: %d\n", t);
            printf("\tlog. adresa: 0x%04x\n", log_adresa);

            dohvati_fizicku_adresu(i, log_adresa);
            // if(i == 0) {
            //     //proizvođač
            //     printf("Proizvođač sam\n");
            //     //Moram staviti N - 1 poruka u zajednički spremnik
            // } else {
            //     //potrošač
            //     printf("Potrošač sam\n");
            //     //Moram uzeti poruku iz zajedničkog spremnika 
            // }
            t++;
            
            sleep(1);
        }
    }

    free(tablica_prevodjenja);
    free(okvir);
    free(disk);
    
}