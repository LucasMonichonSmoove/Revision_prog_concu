#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void)
{
    int pipe1[2], pipe2[2];

    // Création des pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 1er fils : grep "invalid credentials" < server.log
    if (fork() == 0) {
        int input = open("server.log", O_RDONLY);
        if (input == -1) {
            perror("open server.log");
            exit(EXIT_FAILURE);
        }

        dup2(input, STDIN_FILENO);      // stdin <- server.log
        dup2(pipe1[1], STDOUT_FILENO);  // stdout -> pipe1

        close(input);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("grep", "grep", "invalid credentials", NULL);
        perror("execlp grep");
        exit(EXIT_FAILURE);
    }

    // 2e fils : cut -d, -f3
    if (fork() == 0) {
        dup2(pipe1[0], STDIN_FILENO);   // stdin <- pipe1
        dup2(pipe2[1], STDOUT_FILENO);  // stdout -> pipe2

        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("cut", "cut", "-d,", "-f3", NULL);
        perror("execlp cut");
        exit(EXIT_FAILURE);
    }

    // 3e fils : sort > sortie
    if (fork() == 0) {
        int output = open("sortie", O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (output == -1) {
            perror("open sortie");
            exit(EXIT_FAILURE);
        }

        dup2(pipe2[0], STDIN_FILENO);    // stdin <- pipe2
        dup2(output, STDOUT_FILENO);     // stdout -> sortie

        close(output);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("sort", "sort", NULL);
        perror("execlp sort");
        exit(EXIT_FAILURE);
    }

    // Parent : ferme tous les pipes
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Attendre les 3 processus fils
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}