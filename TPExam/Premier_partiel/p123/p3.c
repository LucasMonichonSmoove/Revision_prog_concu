#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include "dijkstra.h"
#include "commun.h"

void t4()
{
    puts("t4");
    sleep(5);
    puts("end t4");
}

int main () {
    int sem_t2 = sem_create(CLE_SEM_T2, 0);
    int sem_t3 = sem_create(CLE_SEM_T3, 0);

    P(sem_t2);   // attendre fin de t2
    P(sem_t3);   // attendre fin de t3

    t4();
    return 0;
}