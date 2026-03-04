#include "../dijkstra.h"      // sem_get(), P(), V()

#include <stdlib.h>           // rand()
#include <unistd.h>           // sleep()
#include <stdio.h>            // printf()

int main()
{
    // semid = arrivée de second
    int semid = sem_get(1);

    // semid2 = arrivée de premier
    int semid2 = sem_get(2);

    // semid3 = arrivée de troisieme (c’est troisieme qui signalera dessus)
    int semid3 = sem_get(3);

    // Travail simulé
    int random = (rand()%10)+1;
    printf("%d\n",random);
    sleep(random);

    // troisieme arrive au rendez-vous
    printf("troisieme attend\n");

    // Signalement : "troisieme est arrivé" (2 jetons pour les 2 autres)
    V(semid3);
    V(semid3);

    // troisieme attend les 2 autres
    P(semid2);   // jeton de premier
    P(semid);    // jeton de second

    // Tous arrivés -> barrière franchie
    printf("troisieme fini\n");
}