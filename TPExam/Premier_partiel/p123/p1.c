#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include "dijkstra.h"
#include "commun.h"

void t1()
{
    puts("t1");
    sleep(10);
    puts("end t1");
}

void t2()
{
    puts("t2");
    sleep(2);
    puts("end t2");
}

int main () {
    int sem_t1 = sem_create(CLE_SEM_T1, 0);
    int sem_t2 = sem_create(CLE_SEM_T2, 0);

    t1();
    V(sem_t1);   // autorise p2 à commencer t3

    t2();
    V(sem_t2);   // signale à p3 que t2 est finie

    return 0;
}