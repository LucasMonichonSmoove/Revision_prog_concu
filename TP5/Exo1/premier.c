#include <stdio.h>      // printf(), perror()
#include <stdlib.h>     // EXIT_SUCCESS / EXIT_FAILURE, srand(), rand()
#include <unistd.h>     // getpid(), sleep()
#include <time.h>       // time()
#include "../dijkstra.h"// sem_get(), P(), V() (wrapper System V)

// Tire un entier aléatoire dans [a; b]
static int rand_in_range(int a, int b)
{
    return a + (rand() % (b - a + 1));
}

int main(void)
{
    // Message au lancement (consigne)
    printf("premier: start\n");

    // Récupère l'identifiant du sémaphore (clé = 1)
    // Si init n'a pas été lancé, le sémaphore n'existe pas => erreur
    int sem = sem_get(1);
    if (sem == -1) {
        perror("premier sem_get");  // affiche l'erreur système (ENOENT etc.)
        return EXIT_FAILURE;
    }

    // Initialise le générateur pseudo-aléatoire
    // time(NULL) change chaque seconde, getpid() rend la graine différente entre processus
    srand((unsigned int)(time(NULL) ^ (unsigned int)getpid()));

    // Génère un temps entre 3 et 5 secondes (consigne)
    int t = rand_in_range(3, 5);

    // Affiche puis attend t secondes (consigne)
    printf("premier: sleep %d sec\n", t);
    sleep((unsigned int)t);

    // Fin de l'attente : on "débloque" second
    // V() = ajoute 1 jeton au sémaphore
    printf("premier: unlock second\n");
    V(sem);

    // Message de fin (consigne)
    printf("premier: exit\n");
    return EXIT_SUCCESS;
}