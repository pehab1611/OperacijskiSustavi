#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

struct timespec t0;
char k_z[3] = "000";
int t_p = 0;
int stog[5] = {-1, -1, -1, -1, -1};
char pokrenut[3] = "000";

void vrijeme();
void obrada_signala(int razina);

void postavi_poc_vrijeme() {
    clock_gettime(CLOCK_REALTIME, &t0);
}

void ispis() {
    vrijeme();
    printf("K_Z = %s, T_P = %d, stog:", k_z, t_p);
    if(stog[0] < 0) {
        printf(" - \n\n");
    } else {
        for(int i = 0; stog[i] >= 0; i++) {
            printf(" %d, reg[%d];", stog[i], stog[i]);
        }
        printf("\n\n");
    }
}

void vrijeme() {
    struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec -= t0.tv_sec;
    t.tv_nsec -= t0.tv_nsec;
    if(t.tv_nsec < 0) {
        t.tv_nsec += 1000000000;
        t.tv_sec--;
    }

    printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec / 1000000);
}

void spavaj(time_t sekundi) {
	struct timespec koliko;
	koliko.tv_sec = sekundi;
	koliko.tv_nsec = 0;

	while (nanosleep(&koliko, &koliko) == -1 && errno == EINTR) {
        vrijeme();
        printf("Nastavlja se izvođenje glavnog programa\n");
        ispis();
        if(k_z != "000") {
            for(int i = 2; i >= 0; i--) {
                if(k_z[i] == '1') {
                    obrada_signala(i + 1);
                    vrijeme();
                    printf("Nastavlja se izvođenje glavnog programa\n");
                    ispis();
                }
            }
        }
    }
}

void obrada_signala(int razina) {
    time_t sekunde = 5;
	struct timespec koliko;
	koliko.tv_sec = sekunde;
	koliko.tv_nsec = 0;

    for(int i = 0; i < 3; i++) {
        pokrenut[i] = '0';
    }
    k_z[razina - 1] = '0';
    pokrenut[razina - 1] = '1';
    int i = 0; 
    while(stog[i] >= 0) {
        i++;
    }
    stog[i] = t_p;
    t_p = razina;

    vrijeme();
    printf("Počela obrada prekida razine %d\n", razina);
    ispis();

    while (nanosleep(&koliko, &koliko) == -1 && errno == EINTR) {
        if(pokrenut[razina - 1] != '1') {
            vrijeme();
            printf("Nastavlja se obrada prekida razine %d\n", razina);
            pokrenut[razina - 1] = '1';
        }
    }

    vrijeme(); 
    printf("Završila obrada prekida razine %d\n", razina);

    if(stog[0] != -1) {
        int i = 4;
        for(; stog[i] < 0; i--);
        t_p = stog[i];
        stog[i] = -1;
    }
    pokrenut[razina - 1] = '0';

    ispis();
}

//prekid razine 1 - SIGUSR1
void obradi_sigusr1(int sig) {
    k_z[0] = '1';
    vrijeme();
    if (1 > t_p) {
        printf("SKLOP: Dogodio se prekid razine 1 i proslijeđuje se procesoru\n");
        ispis();
        obrada_signala(1);
    } else {
        printf("SKLOP: Dogodio se prekid razine 1, ali se pamti i ne proslijeđuje procesoru\n");
        ispis();
    }

    return;
}

//prekid razine 2 - SIGUSR2
void obradi_sigusr2(int sig) {
    k_z[1] = '1';
    vrijeme();
    if (2 > t_p) {
        printf("SKLOP: Dogodio se prekid razine 2 i proslijeđuje se procesoru\n");
        ispis();
        obrada_signala(2);
    } else {
        printf("SKLOP: Dogodio se prekid razine 2, ali se pamti i ne proslijeđuje procesoru\n");
        ispis();
    }

    return;
}

//prekid razine 3 - SIGSYS
void obradi_sigsys(int sig) {
    vrijeme();
    k_z[2] = '1';
    if (3 > t_p) {
        printf("SKLOP: Dogodio se prekid razine 3 i proslijeđuje se procesoru\n");
        ispis();
        obrada_signala(3);
    } else {
        printf("SKLOP: Dogodio se prekid razine 3, ali se pamti i ne proslijeđuje procesoru\n");
    }

    return;
}

void init() {
    struct sigaction act1, act2, act3;

    act1.sa_handler = obradi_sigusr1;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = 0;
    act1.sa_handler = obradi_sigusr1;
    sigaction(SIGUSR1, &act1, NULL);

    act2.sa_handler = obradi_sigusr2;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = 0;
    act2.sa_handler = obradi_sigusr2;
    sigaction(SIGUSR2, &act2, NULL);

    act3.sa_handler = obradi_sigsys;
    sigemptyset(&act1.sa_mask);
    act3.sa_flags = 0;
    act3.sa_handler = obradi_sigsys;
    sigaction(SIGSYS, &act3, NULL);

    clock_gettime(CLOCK_REALTIME, &t0);
}

int main() {

    init();
    int pid = getpid();

    vrijeme();
    printf("Program s PID=%d krenuo s radom\n", pid);
    vrijeme();
    printf("K_Z = %s, T_P = %d, stog: - \n\n", k_z, t_p);

    while(1 == 1) {
        spavaj(100);
    }

    return 0; 
}