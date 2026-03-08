#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int fin = 0;
pid_t fils;

void redirect_fils(int signum) {
    if (signum == SIGUSR1) {
        fin = 1;
    }
}

void redirect_pere(int signum) {
    if (signum == SIGALRM) {
        kill(fils, SIGUSR1);
    }
}

int main(void)
{
    fils = fork();

    if (fils == -1) {
        perror("fork");
        exit(-1);
    }

    if (fils == 0) {
        struct sigaction act_fils;
        memset(&act_fils, 0, sizeof(act_fils));
        act_fils.sa_handler = redirect_fils;
        sigaction(SIGUSR1, &act_fils, NULL);

        while (fin == 0) {
            puts("Le fils travaille");
            sleep(1);
        }

        puts("Le fils se termine normalement");
    } else {
        struct sigaction act_pere;
        memset(&act_pere, 0, sizeof(act_pere));
        act_pere.sa_handler = redirect_pere;
        sigaction(SIGALRM, &act_pere, NULL);

        alarm(10);   // dans 10 secondes, le père reçoit SIGALRM
        pause();     // attend un signal

        wait(NULL);  // attend la fin du fils
    }

    return 0;
}