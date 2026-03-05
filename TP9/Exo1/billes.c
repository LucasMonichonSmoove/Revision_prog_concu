#include <stdlib.h>     // atoi(), exit(), rand(), srand()
#include <stdio.h>      // printf(), puts(), fprintf()
#include <unistd.h>     // sleep()
#include <pthread.h>    // pthread_t, pthread_create/join, mutex
#include <time.h>       // time() pour seed du random
#include <semaphore.h>  // sem_t, sem_init, sem_wait, sem_post

int N;                 // Nombre de threads travailleurs
int nb_max_billes;     // Stock total maximal de billes (capacité)
int nb_billes_dispo;   // Stock courant disponible (variable partagée)
int m;                 // Nombre d'itérations par thread
pthread_mutex_t mutex; // Mutex protégeant nb_billes_dispo (section critique)
sem_t semaphore;       // Sémaphore utilisé pour endormir/réveiller les threads
int canEnd = 0;        // Flag pour arrêter le contrôleur (variable partagée)

// Thread "contrôleur" : vérifie régulièrement l'invariant 0 <= dispo <= max
void *controller(void *unused){
    (void)unused;                // Évite un warning "unused parameter"
    while (canEnd == 0) {        // Tant qu'on n'a pas demandé l'arrêt
        pthread_mutex_lock(&mutex);  // Entre en section critique (lit nb_billes_dispo)

        // Vérifie que le stock n'a pas une valeur impossible
        if (0 > nb_billes_dispo || nb_max_billes < nb_billes_dispo) {
            puts("erreur :/");        // Message d'erreur
            printf("%d\n", nb_billes_dispo); // Affiche la valeur fautive
            exit(-1);                 // Stoppe le programme (brutal mais OK en TP)
        }

        pthread_mutex_unlock(&mutex); // Sort de section critique
        sleep(1);                     // Pause 1s avant la prochaine vérification
    }
    return NULL;                      // Terminaison du thread contrôleur
}

// Demander k billes : bloque si pas assez de billes disponibles
void demander(int nBilles){
    pthread_mutex_lock(&mutex);                 // Protège le test et la modification

    // Tant que le stock disponible est insuffisant, il faut attendre
    while (nBilles > nb_billes_dispo) {
        pthread_mutex_unlock(&mutex);           // Libère le mutex avant de dormir

        // Endort le thread jusqu'à ce qu'un autre rende des billes
        // ⚠️ Attention : ton sémaphore peut "accumuler" des réveils selon init/post (voir remarque plus bas)
        sem_wait(&semaphore);

        pthread_mutex_lock(&mutex);             // Reprend le mutex avant de re-tester
    }

    // Ici : il y a assez de billes, on les prend (section critique)
    nb_billes_dispo -= nBilles;

    pthread_mutex_unlock(&mutex);               // Fin de section critique
}

// Rendre k billes : augmente le stock et réveille des threads en attente
void rendre(int nBilles){
    pthread_mutex_lock(&mutex);         // Protège la mise à jour du stock
    nb_billes_dispo += nBilles;         // Rend n billes (section critique)
    pthread_mutex_unlock(&mutex);       // Libère le mutex

    // Réveille des threads bloqués dans demander()
    // ⚠️ Ici tu fais N "post" (broadcast). Problème classique :
    //    si personne n'attend, les jetons s'accumulent => plus tard sem_wait ne bloquera plus,
    //    et tu risques des boucles inutiles (attente non-passive).
    for (int i = 0; i < N; i++) {
        sem_post(&semaphore);           // Ajoute un jeton au sémaphore (réveil potentiel)
    }
}

// Fonction exécutée par chaque thread travailleur
void *travailleur(void *args){
    int *nb_bille_necessaires = args;   // args pointe vers l'entier k de CE thread

    // Chaque thread répète m fois : demander(k) -> travailler -> rendre(k)
    for (int i = 0; i < m; i++) {
        demander(*nb_bille_necessaires);           // Prend k billes (bloque si besoin)

        printf("travaille avec %d billes\n", *nb_bille_necessaires); // Log

        // Simule le travail (durée aléatoire 1..10 secondes)
        // ⚠️ rand() n'est pas thread-safe : en vrai, préférer rand_r() ou un seed par thread
        sleep((rand() % 10) + 1);

        printf("rend %d billes\n", *nb_bille_necessaires); // Log

        rendre(*nb_bille_necessaires);                      // Rend k billes (réveille)
    }
    return NULL; // Fin du thread travailleur
}

int main(int argc, char **argv){
    // Vérifie qu'on a bien 3 arguments utilisateur
    if (argc < 4) {
        printf("./billes <nombre de threads> <nombre de billes max> <nombre d'itération pour chaque thread>\n");
        exit(-1); // Arrêt si usage incorrect
    }

    // Lecture des arguments de la ligne de commande
    N = atoi(argv[1]);          // Nombre de threads
    nb_max_billes = atoi(argv[2]); // Nombre total de billes max
    m = atoi(argv[3]);          // Nombre d'itérations par thread

    nb_billes_dispo = nb_max_billes; // Au début : toutes les billes sont disponibles

    // Tableau des k (nb de billes nécessaires) pour chaque thread
    // ⚠️ VLA (Variable Length Array) : OK en C99, mais peut poser souci si N très grand
    int billesNecessaires[N];

    srand(time(NULL)); // Initialise le générateur pseudo-aléatoire (seed global)

    // Assigne à chaque thread un besoin k aléatoire entre 1 et nb_max_billes
    for (int i = 0; i < N; i++) {
        billesNecessaires[i] = (rand() % nb_max_billes) + 1;
    }

    // Tableau des identifiants de threads : N travailleurs + 1 contrôleur
    pthread_t thread[N + 1];

    pthread_mutex_init(&mutex, NULL); // Initialise le mutex

    // Initialise le sémaphore
    // ⚠️ Ici tu mets une valeur initiale = N. Cela veut dire qu'au début,
    //    sem_wait() NE BLOQUERA PAS pendant les N premiers appels => pas le comportement attendu.
    //    En général pour "attendre un rendu", on initialise à 0.
    sem_init(&semaphore, 0, N);

    // Crée les N threads travailleurs
    for (int i = 0; i < N; i++) {
        pthread_create(&thread[i], NULL, travailleur, &billesNecessaires[i]);
        // On passe l'adresse de billesNecessaires[i] : chaque thread lit son propre k
    }

    // Crée le thread contrôleur (dernier index)
    pthread_create(&thread[N], NULL, controller, NULL);

    // Attend la fin des N travailleurs
    for (int i = 0; i < N; i++) {
        pthread_join(thread[i], NULL);
    }

    // Indique au contrôleur qu'il peut s'arrêter
    // ⚠️ canEnd n'est pas protégé par mutex/atomic => data race possible (rare mais réel)
    canEnd = 1;

    // Attend la fin du contrôleur
    pthread_join(thread[N], NULL);

    return 0; // Fin du programme
}