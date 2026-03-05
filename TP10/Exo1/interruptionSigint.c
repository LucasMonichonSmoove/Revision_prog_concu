#include <stdio.h>    // printf(), puts()
#include <string.h>   // memset()
#include <unistd.h>   // (pas utilisé ici -> pourrait être retiré)
#include <signal.h>   // sigaction, SIGINT, struct sigaction
#include <stdlib.h>   // exit()

// Handler de signal : fonction appelée automatiquement quand le signal arrive
void redirect(int signum) {
    // signum contient le numéro du signal reçu
    if (signum == SIGINT) {                        // SIGINT = Ctrl+C
        printf("SIGINT(%d) signal received\n", signum); // message de réception
        exit(0);                                   // termine immédiatement le programme
        // NB: exit() force la fin, donc on ne revient pas dans la boucle
    }
}

int main(void)
{
    struct sigaction act;                 // structure décrivant l'action à faire sur un signal

    memset(&act, 0, sizeof(act));         // met à 0 tous les champs (bonne pratique)

    act.sa_handler = redirect;            // on indique le handler à appeler

    sigaction(SIGINT, &act, NULL);        // installe le handler pour SIGINT (Ctrl+C)
    // -> à partir de maintenant, Ctrl+C n'arrête plus "brutalement" :
    //    ça déclenche redirect()

    while (1) {                           // boucle infinie
        puts("affiche");                  // affiche en continu
        // NB: ça spam très vite le terminal, c'est normal
    }

    puts("end");                          // jamais exécuté car while(1) est infini
    return 0;                             // idem : jamais atteint
}