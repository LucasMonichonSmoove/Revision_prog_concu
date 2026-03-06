#include <stdio.h>     // puts()
#include <string.h>    // (pas utilisé ici -> peut être retiré)
#include <unistd.h>    // fork(), sleep()
#include <signal.h>    // kill(), SIGKILL
#include <stdlib.h>    // (pas utilisé ici -> peut être retiré)

int main(void)
{
    pid_t fils = fork();          // Crée un nouveau processus :
                                 // - dans le père, fork() retourne le PID du fils (>0)
                                 // - dans le fils, fork() retourne 0
                                 // - en cas d'erreur, fork() retourne -1

    if (fils != 0) {              // Ici : on est dans le père (ou erreur si fils == -1)
        for (int i = 0; i < 5; i++) {   // Boucle bornée du père (5 itérations)
            if (i == 3) {               // Quand le compteur arrive à 3
                kill(fils, SIGKILL);    // Le père envoie SIGKILL au fils => le fils est tué (non interceptable)
            }
            puts("père");               // Affiche "père"
            sleep(1);                   // Attend 1 seconde (rythme 1 affichage / seconde)
        }
    } else {                     // Ici : on est dans le processus fils (fork() a renvoyé 0)
        while (1) {              // Boucle infinie du fils
            puts("fils");        // Affiche "fils"
            sleep(1);            // Attend 1 seconde
        }
    }

    return 0;                    // Fin du programme (le père termine après sa boucle)
}