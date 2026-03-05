#include <stdlib.h>    // atoi()
#include <stdio.h>     // printf()
#include <unistd.h>    // getpid()
#include <pthread.h>   // pthread_t, pthread_create/join, mutex
#include <time.h>      // (⚠️ ici pas utilisé dans ta version, tu peux enlever)

int N;                 // nombre total d'essais (global partagé en lecture)
int nbThreads;         // nombre de threads (global partagé en lecture)

// nombre de d'essais qui sont dans le cercle
int qteHit = 0;        // compteur global partagé (écriture concurrente => mutex obligatoire)

void *calc(void *args){
    // récupère le mutex
    pthread_mutex_t *mutex = args; // args pointe vers le mutex créé dans main()

    // static initialise la variable une seule fois, elle est partagée ensuite entre tous les threads.
    // => donc accès concurrent possible (il faut protéger avec mutex, ce que tu fais)
    static int th_id = 0;

    // On protège la mise à jour de th_id pour attribuer un id unique à chaque thread
    pthread_mutex_lock(mutex);
    th_id++;
    pthread_mutex_unlock(mutex);

    int hit = 0;        // compteur local (privé au thread) : pas besoin de mutex

    // définit les deux nombre flotants qui seront utilisés pour les coordonnées
    float x, y;         // variables locales au thread (OK)

    int it = N / nbThreads;  // nb d'itérations par thread (⚠️ si N pas divisible, on perd N%nbThreads tirages)

    // seed pour rand_r() :
    // - getpid() : identique pour tous les threads (même processus)
    // - th_id : différencie un peu les threads
    // ⚠️ `uint` n'est pas standard C, mieux: unsigned int
    uint seed = getpid() + th_id;

    for(int i = 0; i < it; i++){
        // rand_r(&seed) : version réentrante (thread-safe) de rand() :
        // le seed est mis à jour à chaque appel, et est local au thread
        x = ((float)rand_r(&seed) / (float)(RAND_MAX));
        y = ((float)rand_r(&seed) / (float)(RAND_MAX));

        // test "dans le cercle" : x² + y² <= 1
        if(x*x + y*y < 1){
            hit++;  // incrémente le hit local
        }
    }

    // Ajoute le compte local au compte global
    pthread_mutex_lock(mutex);  // section critique : écriture sur qteHit
    qteHit += hit;              // accumulation globale
    pthread_mutex_unlock(mutex);

    return NULL; // fin du thread
}

int main(int argc, char **argv){
    if(argc < 3){      // on attend N et nbThreads
        exit(-1);      // quitte sans message si mauvais usage
    }

    N = atoi(argv[1]);         // nombre total d'essais
    nbThreads = atoi(argv[2]); // nombre de threads

    // création des threads
    pthread_t thread[nbThreads]; // tableau d'identifiants de threads (VLA, OK si nbThreads raisonnable)

    // création du mutex
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL); // initialise le mutex

    // Crée nbThreads threads, tous exécutent calc()
    for(int i = 0; i < nbThreads; i++){
        pthread_create(&thread[i], NULL, calc, &mutex); // on passe l'adresse du mutex à chaque thread
    }

    // Attend la fin de tous les threads
    for(int i = 0; i < nbThreads; i++){
        pthread_join(thread[i], NULL);
    }

    // pi ≈ 4 * hit / N
    printf("pi:%f\n", 4 * ((float)qteHit / (float)N));

    return 0; // fin programme
}