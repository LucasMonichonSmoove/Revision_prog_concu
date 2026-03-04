#include <stdio.h>       // printf(), perror()
#include <stdlib.h>      // EXIT_SUCCESS / EXIT_FAILURE, srand(), rand()
#include <unistd.h>      // getpid(), sleep()
#include <time.h>        // time()
#include "../dijkstra.h" // sem_get(), P(), V()

// Tire un entier aléatoire dans [a; b]
static int rand_in_range(int a, int b)
{
    return a + (rand() % (b - a + 1));
}

int main(void)
{
    // Message au lancement (consigne)
    printf("second: start\n");

    // Récupère l'identifiant du sémaphore (clé = 1)
    int sem = sem_get(1);
    if (sem == -1) {
        perror("second sem_get");
        return EXIT_FAILURE;
    }

    // second doit attendre que premier ait fini son attente
    // P() = prend 1 jeton ; s'il n'y en a pas (sémaphore à 0) => BLOQUE
    printf("second: waiting premier...\n");
    P(sem);

    // Si on arrive ici, c'est que premier a fait V() (donc premier a fini son attente)
    // (optionnel mais super clair pour débug)
    // printf("second: unlocked!\n");

    // Initialise le random (après le déblocage, peu importe mais logique)
    srand((unsigned int)(time(NULL) ^ (unsigned int)getpid()));

    // Génère un temps entre 1 et 4 secondes (consigne)
    int t = rand_in_range(1, 4);

    // Attend ce temps puis termine (consigne)
    printf("second: sleep %d sec\n", t);
    sleep((unsigned int)t);

    printf("second: exit\n");
    return EXIT_SUCCESS;
}