#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "dijkstra.h"
#include "commun.h"

int main () {
    int sem_t1 = sem_create(CLE_SEM_T1, 0);
    int sem_t2 = sem_create(CLE_SEM_T2, 0);
    int sem_t3 = sem_create(CLE_SEM_T3, 0);

    if (fork() == 0)
    {
        execlp("./p1", "./p1", NULL);
        perror("execlp p1");
        exit(1);
    }
    if (fork() == 0)
    {
        execlp("./p2", "./p2", NULL);
        perror("execlp p2");
        exit(1);
    }
    if (fork() == 0)
    {
        execlp("./p3", "./p3", NULL);
        perror("execlp p3");
        exit(1);
    }

    wait(NULL);
    wait(NULL);
    wait(NULL);

    sem_delete(sem_t1);
    sem_delete(sem_t2);
    sem_delete(sem_t3);

    return 0;
}