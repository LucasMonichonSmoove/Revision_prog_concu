#include "../dijkstra.h"   // sem_get(), P(), V()
#include <stdlib.h>        // atoi(), rand(), exit()
#include <unistd.h>        // sleep()
#include <stdio.h>         // printf()
#include <sys/ipc.h>       // types/constantes IPC
#include <sys/shm.h>       // shmget(), shmat(), shmdt()

int main(int argc, char **argv)
{
    // Vérifie qu'on a bien l'argument N (nombre de processus attendus)
    if(argc < 2){
        exit(-1);          // quitte si pas d'argument (mieux : afficher un usage)
    }

    // Convertit l'argument texte en int : N = nombre de processus au rendez-vous
    int nbProcessus = atoi(argv[1]);

    // Récupère les sémaphores (créés par init)
    // semid = barrière (clé=1)
    int semid = sem_get(1);

    // mutex = exclusion mutuelle (clé=2) pour protéger l’accès à *mem
    int mutex = sem_get(2);

    // Récupère l'identifiant de la mémoire partagée (même clé que init : 5942)
    // flags = 0 => on demande juste à récupérer un segment existant
    int shmid = shmget(5942, sizeof(int), 0);

    // Attache la mémoire partagée dans ce processus
    // mem pointe sur l'int partagé : le compteur d’arrivées
    int* mem = shmat(shmid, NULL, 0);

    // Simule un "travail" avant d'arriver au rendez-vous :
    // attend entre 1 et 20 secondes (aléatoire)
    // (attention : sans srand, plusieurs exécutions peuvent avoir le même "random")
    int random = (rand()%20)+1;
    sleep(random);

    // Prend le mutex : interdit aux autres processus de modifier *mem en même temps
    P(mutex);

    // On indique qu'on est arrivé au rendez-vous : on incrémente le compteur partagé
    *mem += 1;

    // Si on est le dernier à arriver (compteur == N),
    // alors on libère la barrière pour tout le monde :
    // on fait V(semid) N fois pour donner N jetons
    if(*mem == nbProcessus)
    {
        for(int i = 0; i < nbProcessus; i++){
            V(semid);
        }
    }

    // Affiche le nombre total d'arrivés (valeur du compteur partagé)
    // (affichage peut être intercalé entre processus, c’est normal)
    printf("%d arrivé\n", mem[0]);

    // Rend le mutex : autorise les autres processus à toucher *mem
    V(mutex);

    // Barrière : tous les processus attendent ici tant que le dernier n'a pas libéré
    // Si le dernier a fait V() N fois, chaque processus prendra 1 jeton et passera
    P(semid);

    // Après la barrière : tous peuvent continuer indépendamment
    printf("reparti\n");

    // Détache la mémoire partagée dans ce processus
    shmdt(mem);
}