#include <stdio.h>     // puts()
#include <string.h>    // memset()
#include <unistd.h>    // (pas utilisé ici -> peut être retiré)
#include <signal.h>    // sigaction(), SIGINT, struct sigaction
#include <stdlib.h>    // (pas utilisé ici -> peut être retiré)

// Variable globale qui contrôle l'arrêt de la boucle
// 0 = faux (on continue), 1 = vrai (on s'arrête)
// (version "béton": utiliser volatile sig_atomic_t)
int fin = 0;

// Handler du signal : appelé automatiquement quand SIGINT (Ctrl+C) arrive
void redirect(int signum) {
    if (signum == SIGINT) { // si on reçoit Ctrl+C
        fin = 1;            // on demande l'arrêt "propre" en sortant de la boucle
    }
}

int main(void)
{
    struct sigaction act;                 // structure pour configurer un handler de signal

    memset(&act, 0, sizeof(act));         // initialise tous les champs à 0 (bonne pratique)

    act.sa_handler = redirect;            // on indique quelle fonction appeler à la réception du signal

    sigaction(SIGINT, &act, NULL);        // installe le handler pour SIGINT (Ctrl+C)

    // Boucle principale : tourne tant que fin reste à 0
    while (fin == 0) {
        puts("affiche");                  // affichage répété
        // usleep(100000);                // optionnel : évite de saturer le terminal (100 ms)
    }

    // Quand Ctrl+C arrive, redirect() met fin=1 => on sort de la boucle et on finit proprement
    puts("end");

    return 0;                             // fin normale du programme
}