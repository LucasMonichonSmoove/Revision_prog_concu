#include <stdlib.h>   // atoi(), srand(), rand(), exit()
#include <stdio.h>    // printf()
#include <unistd.h>   // sleep() (⚠️ ici pas utilisé -> peut être supprimé)
#include <time.h>     // time() pour la seed

// Simple fonction qui met un float à la puissance 2
void carre(float *n){
    *n = (*n)*(*n);   // déréférence le pointeur et remplace la valeur par n²
}

int main(int argc, char **argv){
    if(argc < 2){      // on attend au moins 1 argument (N)
        exit(-1);      // quitte le programme si mauvais usage (pas de message)
    }

    int N = atoi(argv[1]);   // conversion de l’argument 1 en entier : N = nb de tirages

    // définit la seed du random sur le time
    srand(time(NULL));       // initialise le générateur pseudo-aléatoire global rand()

    // définit les deux nombre flotants qui seront utilisés pour les coordonnées
    float x, y;              // variables locales pour stocker les coordonnées

    // nombre de d'essais qui sont dans le cercle
    int qteHit = 0;          // compteur local : nb de points dans le quart de cercle

    for(int i = 0; i < N; i++){
        // Tire x et y uniformément dans [0,1]
        // rand() ∈ [0, RAND_MAX], on normalise en float
        x = ((float)rand() / (float)(RAND_MAX));
        y = ((float)rand() / (float)(RAND_MAX));

        carre(&x);           // x = x²
        carre(&y);           // y = y²

        // Test "dans le quart de disque de rayon 1"
        // Condition math : x² + y² <= 1
        // Ici comme x et y ont déjà été mis au carré : (x + y) < 1
        if(x + y < 1){
            qteHit++;         // si le point est dans le cercle, on incrémente hit
        }
    }

    // pi ≈ 4 * hit / N
    printf("pi:%f\n", 4 * ((float)qteHit / (float)N));

    return 0;                // fin du programme
}