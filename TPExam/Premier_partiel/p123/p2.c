#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include "dijkstra.h"
#include "commun.h"

void t3()
{
    puts("t3");
    sleep(7);
    puts("end t3");
}

int main () {
    int sem_t1 = sem_create(CLE_SEM_T1, 0);
    int sem_t3 = sem_create(CLE_SEM_T3, 0);

    P(sem_t1);   // attendre fin de t1
    t3();
    V(sem_t3);   // signaler à p3 que t3 est finie

    return 0;
}