#include "../dijkstra.h"      // sem_get(), P(), V()

#include <stdlib.h>           // rand()
#include <unistd.h>           // sleep()
#include <stdio.h>            // printf()

int main()
{
    // semid = sémaphore d’arrivée de second (c’est second qui va signaler dessus)
    int semid = sem_get(1);

    // semid2 = sémaphore d’arrivée de premier (premier signalera dessus)
    int semid2 = sem_get(2);

    // semid3 = sémaphore d’arrivée de troisieme (troisieme signalera dessus)
    int semid3 = sem_get(3);

    // Travail simulé : temps 1..10
    int random = (rand()%10)+1;
    printf("%d\n",random);
    sleep(random);

    // second arrive au rendez-vous
    printf("second attend\n");

    // Signalement : "second est arrivé"
    // 2 jetons car 2 autres processus doivent recevoir chacun 1 jeton.
    V(semid);
    V(semid);

    // second attend ensuite les 2 autres :
    // attendre un jeton de premier (arrivée de premier)
    P(semid2);

    // attendre un jeton de troisieme (arrivée de troisieme)
    P(semid3);

    // Si on arrive ici, les 3 sont arrivés -> barrière franchie
    printf("second fini\n");
}