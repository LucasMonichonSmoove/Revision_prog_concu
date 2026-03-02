#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int len = strlen(argv[i]);

        /* +1 pour le '\0' final */
        char result[len + 1];

        /* le '\0' doit être à l'index len */
        result[len] = '\0';

        for (int j = len; j > 0; j--) {
            result[len - j] = argv[i][j - 1];
        }

        /* pas de \n à chaque mot, espace entre mots */
        printf("%s", result);
        if (i < argc - 1) {
            printf(" ");
        }
    }

    printf("\n");
    return 0;
}
