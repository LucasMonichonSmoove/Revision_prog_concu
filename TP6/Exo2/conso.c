#include "../dijkstra.h"   // sem_get(), P(), V()
#include "data.h"          // BUFFERSIZE, BUFFERKEY

#include <stdlib.h>        // (pas utilisé ici mais ok)
#include <unistd.h>        // (sleep/usleep possibles)
#include <stdio.h>         // printf()
#include <sys/ipc.h>       // IPC
#include <sys/shm.h>       // shmget(), shmat(), shmdt()

int main()
{
    // Récupère le sémaphore mutex (protège buffer + head + count)
    int mutexbuffer = sem_get(1);

    // Sémaphore de "places libres" (empty)
    // Ici conso ne l’attend pas, il le libère après lecture
    int semwrite = sem_get(2);

    // Sémaphore de "données disponibles" (full)
    // Conso doit attendre dessus si buffer vide
    int semread = sem_get(3);

    // Récupère l’identifiant de la SHM existante (créée par init)
    int shmidbuffer = shmget(BUFFERKEY,(BUFFERSIZE+2)*sizeof(int),0);

    // Attache la SHM dans l’espace mémoire du processus
    int* buffer = shmat(shmidbuffer,NULL,0);

    while(1){
        // 1) Attend qu’il y ait au moins 1 donnée à lire
        // Si buffer vide => semread==0 => P bloque
        P(semread);

        // 2) Entrée en section critique sur le buffer (pour FIFO + multi conso)
        P(mutexbuffer);

        // buffer[BUFFERSIZE] = head (index de la prochaine donnée à lire)
        // On lit la donnée à cet index
        int value = buffer[buffer[BUFFERSIZE]];

        // Affichage debug
        printf("reçu %d depuis l'index %d\n",value, buffer[BUFFERSIZE]);

        // Avance le head (FIFO) : on passe à l’index suivant (circulaire)
        buffer[BUFFERSIZE] = (buffer[BUFFERSIZE]+1)%BUFFERSIZE;

        // Décrémente le nombre d’éléments présents dans le buffer
        buffer[BUFFERSIZE+1] -= 1;

        // 3) Sort de la section critique
        V(mutexbuffer);

        // 4) Rend une place libre => un producteur peut écrire
        V(semwrite);
    }

    // Détache la SHM (jamais atteint ici car while(1))
    shmdt(buffer);
}