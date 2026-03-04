#include "../dijkstra.h"   // sem_create()
#include "data.h"          // BUFFERSIZE, BUFFERKEY

#include <stdio.h>         // printf (si tu veux)
#include <sys/ipc.h>       // IPC_CREAT, IPC_EXCL
#include <sys/shm.h>       // shmget()

int main(void){
    // Semaphore 1 = MUTEX (protection du buffer en mémoire partagée)
    // Init à 1 => une seule entrée possible en section critique
    sem_create(1,1);

    // Semaphore 2 = WRITE / EMPTY (nombre de places libres dans le buffer)
    // Init à BUFFERSIZE => au début le buffer est vide, donc toutes les places sont libres
    sem_create(2,BUFFERSIZE);

    // Semaphore 3 = READ / FULL (nombre d’éléments disponibles à lire)
    // Init à 0 => au début aucun élément n’est disponible
    sem_create(3,0);

    // Création du segment de mémoire partagée "buffer"
    // Taille = BUFFERSIZE cases de données + 2 cases méta (head + count)
    // IPC_EXCL => échoue si le segment existe déjà (d'où clean obligatoire avant init)
    shmget(BUFFERKEY,(BUFFERSIZE+2)*sizeof(int),IPC_CREAT|IPC_EXCL|0600);

    // ⚠️ IMPORTANT (manquant dans ton init actuel pour un TP nickel) :
    // il faut initialiser les 2 cases méta dans le buffer :
    // buffer[BUFFERSIZE] = 0;   // head (index de lecture)
    // buffer[BUFFERSIZE+1] = 0; // count (nb d'éléments)
    // sinon elles peuvent contenir n'importe quoi au premier run.
}