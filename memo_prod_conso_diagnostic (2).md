# Mémo Partiel -- Diagnostic Producteur / Consommateur (Mémoire Partagée + Sémaphores)

## Contexte typique d'exercice

On rencontre souvent au partiel un programme composé de : - un
**producteur** - un **consommateur** - une **mémoire partagée** - des
**sémaphores System V** - un **buffer circulaire de taille MEM**

Le producteur écrit des données dans le buffer et le consommateur les
lit.

Exemple d'objectif : abcdefghijklmnopqrstuvwxyz

Mais un bug de synchronisation peut produire une sortie incorrecte comme
: defdefjkijkopnoptustuyzxyz

Ce type de résultat indique presque toujours un **problème de
synchronisation ou d'initialisation des sémaphores**.

------------------------------------------------------------------------

# 1. Structure classique Producteur / Consommateur

Buffer circulaire :

buffer\[i % MEM\]

Deux sémaphores sont utilisés :

  Sémaphore   Rôle
  ----------- -----------------------------
  sem_prod    nombre de **cases libres**
  sem_conso   nombre de **cases pleines**

------------------------------------------------------------------------

# 2. Valeurs initiales correctes

Au démarrage :

buffer vide

Donc :

sem_prod = MEM\
sem_conso = 0

Pourquoi ?

-   toutes les cases sont libres
-   aucune donnée n'est encore produite

------------------------------------------------------------------------

# 3. Schéma correct des opérations

## Producteur

P(sem_prod)\
écrire dans le buffer\
V(sem_conso)

## Consommateur

P(sem_conso)\
lire dans le buffer\
V(sem_prod)

------------------------------------------------------------------------

# 4. Bug classique rencontré en partiel

Exemple :

#define MEM 3\
#define SEM 5

et dans le code :

sem_create(CLE_SEM_PROD, SEM)

Problème :

Le sémaphore `sem_prod` représente le **nombre de cases libres**.

Mais ici il est initialisé à **5** alors que le buffer ne contient que
**3 cases**.

Conséquence :

Le producteur peut écrire **5 fois** avant d'être bloqué.

Simulation :

  écriture   case
  ---------- --------------
  a          0
  b          1
  c          2
  d          0 (écrase a)
  e          1 (écrase b)

Le consommateur lit alors des valeurs incorrectes.

------------------------------------------------------------------------

# 5. Comment diagnostiquer ce bug en partiel

Suivre cette méthode :

## Étape 1 -- Identifier les ressources

Chercher : #define MEM ...

Cela correspond à la **taille du buffer**.

------------------------------------------------------------------------

## Étape 2 -- Comprendre le rôle des sémaphores

Traduire mentalement :

sem_prod = nombre de places libres\
sem_conso = nombre de cases pleines

------------------------------------------------------------------------

## Étape 3 -- Vérifier l'initialisation

Si le buffer fait MEM cases :

sem_prod doit être initialisé à MEM\
sem_conso doit être initialisé à 0

------------------------------------------------------------------------

## Étape 4 -- Vérifier l'ordre P/V

Producteur :

P(libres) → écrire → V(pleines)

Consommateur :

P(pleines) → lire → V(libres)

------------------------------------------------------------------------

## Étape 5 -- Vérifier l'indexation

Dans un buffer circulaire :

index % MEM

------------------------------------------------------------------------

# 6. Indices révélateurs d'un problème

  Symptôme             Cause probable
  -------------------- ---------------------------
  lettres manquantes   écrasement dans le buffer
  lettres répétées     désynchronisation
  ordre incorrect      mauvais sémaphore
  blocage total        deadlock

Dans le cas étudié :

defdefjkijkopnoptustuyzxyz

→ indique **écrasement de données dans le buffer**.

------------------------------------------------------------------------

# 7. Correction

Initialiser correctement le sémaphore du producteur :

sem_create(CLE_SEM_PROD, MEM);\
sem_create(CLE_SEM_CONSO, 0);

------------------------------------------------------------------------

# 8. Problème supplémentaire possible : IPC persistantes

Les sémaphores System V restent parfois en mémoire après l'exécution.

Vérification :

ipcs -s

Suppression :

ipcrm -S CLE_SEM_PROD\
ipcrm -S CLE_SEM_CONSO\
ipcrm -M CLE_MEM

Sinon un ancien sémaphore peut conserver une mauvaise valeur.

------------------------------------------------------------------------

# 9. Règle d'or pour le partiel

Dans un **producteur / consommateur avec buffer borné** :

sémaphore des cases libres = taille du buffer\
sémaphore des cases pleines = 0

------------------------------------------------------------------------

# 10. Phrase parfaite pour la copie

Le sémaphore `sem_prod` représente le nombre de cases libres dans le
buffer partagé.\
Il doit donc être initialisé à `MEM`, la taille du buffer.\
Dans le code fourni, il était initialisé à `SEM=5` alors que le buffer
contient `MEM=3` cases, ce qui permet au producteur d'écrire trop
d'éléments avant blocage et d'écraser des données non encore consommées.
