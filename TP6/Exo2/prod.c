#include "../dijkstra.h"   // sem_get(), P(), V()
#include "data.h"          // BUFFERSIZE, BUFFERKEY

#include <stdlib.h>        // rand()
#include <unistd.h>        // sleep()
#include <stdio.h>         // printf()
#include <sys/ipc.h>
#include <sys/shm.h>       // shmget(), shmat(), shmdt()

int main()
{
    // Sémaphore mutex pour protéger la mémoire partagée
    int mutexbuffer = sem_get(1);

    // Sémaphore "places libres" (empty)
    // Prod attend dessus si buffer plein
    int semwrite = sem_get(2);

    // Sémaphore "données dispo" (full)
    // Prod le libère après écriture
    int semread = sem_get(3);

    // Récupère et attache la mémoire partagée (buffer)
    int shmidbuffer = shmget(BUFFERKEY,(BUFFERSIZE+2)*sizeof(int),0);
    int* buffer = shmat(shmidbuffer,NULL,0);

    while(1){
        // Génère une valeur à produire (0..9) + un délai
        int random = (rand()%10);
        sleep(random);

        // 1) Attend une place libre
        // Si buffer plein => semwrite==0 => P bloque
        P(semwrite);

        // 2) Section critique (multi producteurs)
        P(mutexbuffer);

        // buffer[BUFFERSIZE] = head (index de lecture)
        // buffer[BUFFERSIZE+1] = count (nb d’éléments présents)
        // L’index d’écriture FIFO (tail) = (head + count) % BUFFERSIZE
        int index = (buffer[BUFFERSIZE] + buffer[BUFFERSIZE+1])%BUFFERSIZE;

        // Écrit la donnée dans le buffer
        buffer[index] = random;

        // Affichage debug
        printf("envoyé %d sur l'index %d\n", random, index);

        // Incrémente le nombre d’éléments présents
        buffer[BUFFERSIZE+1] += 1;

        // 3) Sort de la section critique
        V(mutexbuffer);

        // 4) Signale qu’une donnée est dispo à lire
        V(semread);
    }

    // Détache SHM (jamais atteint ici)
    shmdt(buffer);
}