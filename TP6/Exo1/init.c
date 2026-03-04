#include <stdio.h>        // perror(), printf()
#include "../dijkstra.h"  // sem_create()
#include <sys/ipc.h>      // types/constantes IPC
#include <sys/shm.h>      // shmget(), shmat(), shmdt()

int main(void)
{
    // Crée (ou récupère si déjà existant) un segment de mémoire partagée
    // clé = 5942, taille = 1 int, droits = 0600 (owner read/write)
    int shmid = shmget(5942, sizeof(int), IPC_CREAT | 0600);

    // Si shmget échoue, on affiche l’erreur système et on sort avec code 1
    if (shmid == -1) { perror("shmget"); return 1; }

    // Attache (mappe) la mémoire partagée dans l’espace d’adressage du processus
    // -> mem pointe sur l'int partagé
    int *mem = shmat(shmid, NULL, 0);

    // shmat renvoie (void*)-1 en cas d’erreur
    if (mem == (void*)-1) { perror("shmat"); return 1; }

    // IMPORTANT : reset du compteur partagé avant chaque test
    // Sans ça, le compteur garde l’ancienne valeur et la barrière peut ne plus se libérer.
    *mem = 0;              // ✅ reset compteur

    // Détache la mémoire partagée (on n’en a plus besoin dans init)
    shmdt(mem);

    // Crée le sémaphore de barrière (clé=1) avec 0 jeton au départ :
    // -> tous les processus devront attendre dessus tant que le dernier n’a pas libéré.
    sem_create(1, 0);       // barrière

    // Crée le sémaphore mutex (clé=2) avec 1 jeton :
    // -> permet l'exclusion mutuelle sur la variable partagée *mem
    sem_create(2, 1);       // mutex

    // Fin OK
    return 0;
}