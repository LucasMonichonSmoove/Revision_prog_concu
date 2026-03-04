#include "../dijkstra.h"   // sem_get(), sem_delete()
#include "data.h"          // BUFFERKEY, BUFFERSIZE (si besoin)

#include <sys/ipc.h>       // IPC_RMID
#include <sys/shm.h>       // shmctl(), shmget()
#include <stdio.h>         // perror()

int main()
{
    // ⚠️ PROBLÈME : shmctl() attend un SHMID (identifiant), pas une KEY.
    // Ici tu passes BUFFERKEY => ce n’est pas le bon paramètre.
    // La bonne démarche :
    //   int shmid = shmget(BUFFERKEY, (BUFFERSIZE+2)*sizeof(int), 0600);
    //   shmctl(shmid, IPC_RMID, NULL);

    shmctl(BUFFERKEY, IPC_RMID, NULL);  // ❌ pas correct : BUFFERKEY n'est pas un shmid

    // Supprime le sémaphore mutex
    sem_delete(sem_get(1));

    // Supprime le sémaphore "places libres" (write/empty)
    sem_delete(sem_get(2));

    // Supprime le sémaphore "éléments dispos" (read/full)
    sem_delete(sem_get(3));
}