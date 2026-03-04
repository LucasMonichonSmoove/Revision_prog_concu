#include "../dijkstra.h"      // Fournit sem_get(), P(), V() (sémaphores System V via ton wrapper)

#include <stdlib.h>           // rand() (et éventuellement srand() si tu l’ajoutes)
#include <unistd.h>           // sleep()
#include <stdio.h>            // printf()

int main()
{
    // Récupère l'identifiant (semid) du sémaphore associé à la clé 1
    // Ici, semid = "sémaphore d’arrivée de second" (c’est lui qui fera V(semid))
    int semid = sem_get(1);

    // Récupère l'identifiant du sémaphore associé à la clé 2
    // Ici, semid2 = "sémaphore d’arrivée de premier" (premier signalera sur celui-ci)
    int semid2 = sem_get(2);

    // Récupère l'identifiant du sémaphore associé à la clé 3
    // Ici, semid3 = "sémaphore d’arrivée de troisieme" (il fera V(semid3))
    int semid3 = sem_get(3);

    // Simule un traitement : génère un temps aléatoire entre 1 et 10
    int random = (rand()%10)+1;

    // Affiche ce temps (utile pour voir combien de temps ce processus va "travailler")
    printf("%d\n",random);

    // Simule le travail avant d'arriver au rendez-vous
    sleep(random);

    // On est arrivé au rendez-vous (fin du "travail" du premier)
    printf("premier attend\n");

    // Signalement : "premier est arrivé"
    // Pourquoi 2 fois ?
    // Parce qu'il y a 2 autres processus (second et troisieme) qui doivent chacun pouvoir prendre 1 jeton.
    // => on dépose 2 jetons pour débloquer les 2 autres quand ils feront P(semid2).
    V(semid2);
    V(semid2);

    // Maintenant premier attend les 2 autres :
    // P(semid) = attendre un jeton envoyé par second (son arrivée)
    P(semid);

    // P(semid3) = attendre un jeton envoyé par troisieme (son arrivée)
    P(semid3);

    // Si on arrive ici, ça veut dire :
    // - second a signalé son arrivée (V sur semid)
    // - troisieme a signalé son arrivée (V sur semid3)
    // Donc les 3 sont au rendez-vous => on peut repartir
    printf("premier fini\n");
}