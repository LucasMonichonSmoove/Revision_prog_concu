#include <stdio.h>     // puts()
#include <string.h>    // memset()
#include <unistd.h>    // fork(), sleep()
#include <signal.h>    // sigaction(), kill(), SIGUSR1, SIGUSR2
#include <stdlib.h>    // (pas utilisé ici -> peut être retiré)


// Handler (fonction d'interception) appelé quand un signal arrive au processus fils
void redirect(int signum) {
    // signum contient le numéro du signal reçu
    if (signum == SIGUSR1) {               // si on reçoit SIGUSR1
        puts("interception SIGUSR1");      // on affiche un message
        // NB: selon l'énoncé, on pourrait afficher "fils" ici (ou déclencher un affichage)
    }
}

int main(void)
{
    pid_t fils = fork();                   // création du processus fils

    if (fils != 0) {                       // ----- Processus père (fork() retourne PID du fils > 0) -----

        // Le père fait des affichages dans une boucle for (ici 1..6)
        for (int i = 1; i < 7; i++) {

            // À l'itération 3 et 5, le père envoie SIGUSR1 au fils
            if (i == 3 || i == 5) {
                kill(fils, SIGUSR1);       // envoi du signal SIGUSR1 au fils
            }

            puts("père");                  // affichage du père
            sleep(1);                      // attend 1 seconde
        }

        // Après la boucle, le père envoie SIGUSR2 au fils pour signaler la fin
        kill(fils, SIGUSR2);

    } else {                               // ----- Processus fils (fork() retourne 0) -----

        // Installation d'un handler SIGUSR1 dans le fils
        struct sigaction act;
        memset(&act, 0, sizeof(act));      // initialise la structure à 0
        act.sa_handler = redirect;         // associe la fonction redirect comme handler

        // Installe le handler pour SIGUSR1 uniquement
        sigaction(SIGUSR1, &act, NULL);

        // Boucle infinie du fils
        while (1) {
            puts("fils");                  // affichage régulier du fils (toutes les secondes)
            sleep(1);
        }

        // NB: ce code n'est jamais atteint (boucle infinie)
        // Dans ta version, le fils sera stoppé quand il recevra SIGUSR2,
        // car SIGUSR2 n'est pas intercepté ici => action par défaut = terminer le processus.
    }

    return 0;                              // fin du programme (le père termine après la boucle)
}