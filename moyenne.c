#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("Impossible");
        return 1;
    }
    float avg = 0;
    for (int i = 1; i < argc; i++) {
        float nombre;
        int nb_success = sscanf(argv[i], "%f", &nombre);
        if (nb_success <1){
                puts("Impossible");
                return 2;
        }
        if (nombre < 0 || nombre > 20){
                puts("Impossible");
                return 2;
        }
        avg += nombre;               
    }
    avg = avg / (argc - 1);
    printf("%.2f\n", avg);
    return 0;
}
