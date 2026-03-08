import multiprocessing as mp
# On importe le module multiprocessing de Python.
# Il permet de lancer plusieurs processus en parallèle.
# Ici, il servira à faire courir plusieurs chevaux en même temps.

from multiprocessing import Process
# On importe directement la classe Process pour créer plus facilement
# des processus avec Process(...).

import os, time, random, ctypes, sys
# os     : fonctions liées au système d’exploitation
# time   : pour faire des pauses avec sleep()
# random : pour générer des vitesses aléatoires pour les chevaux
# ctypes : pour typer certaines variables partagées (ex : booléen C)
# sys    : accès à certains paramètres système (pas vraiment utilisé ici directement)

# ==========================================================
# ========== Codes VT100 : gestion de l'affichage ==========
# ==========================================================
# Ces constantes sont des séquences spéciales envoyées au terminal
# pour effacer l'écran, déplacer le curseur, changer les couleurs, etc.

CLEARSCR = "\x1B[2J\x1B[;H"
# \x1B = caractère ESC
# [2J  = efface tout l’écran
# [;H  = replace le curseur en haut à gauche

CURSON   = "\x1B[?25h"
# Rend le curseur visible

CURSOFF  = "\x1B[?25l"
# Rend le curseur invisible

CL_RED="\033[22;31m"
CL_GREEN="\033[22;32m"
CL_BROWN="\033[22;33m"
CL_BLUE="\033[22;34m"
CL_MAGENTA="\033[22;35m"
CL_CYAN="\033[22;36m"
CL_GRAY="\033[22;37m"
CL_WHITE="\033[01;37m"
# Différentes couleurs ANSI / VT100 pour afficher les chevaux
# dans des couleurs différentes.

NORMAL = "\x1B[0m"
# Réinitialise le style du terminal :
# couleur normale, pas de gras, etc.


def effacer_ecran():
    print(CLEARSCR, end='', flush=True)
# Affiche la séquence qui efface tout l’écran.
# end='' évite le retour à la ligne.
# flush=True force l’affichage immédiat dans le terminal.

def curseur_invisible():
    print(CURSOFF, end='', flush=True)
# Masque le curseur pour que l’affichage soit plus propre pendant la course.

def curseur_visible():
    print(CURSON, end='', flush=True)
# Réaffiche le curseur à la fin du programme.

def move_to(lig, col):
    print(f"\033[{lig};{col}f", end='', flush=True)
# Déplace le curseur du terminal à la ligne "lig" et colonne "col".
# Exemple : move_to(5, 10) place l’écriture à la ligne 5, colonne 10.

def erase_line_from_beg_to_curs():
    print("\033[1K", end='', flush=True)
# Efface la ligne depuis son début jusqu’à la position actuelle du curseur.
# Ici c’est utilisé pour "nettoyer" avant de redessiner un cheval.

def en_couleur(c):
    print(c, end='', flush=True)
# Affiche une séquence de couleur ANSI.
# Exemple : en_couleur(CL_RED) met le texte suivant en rouge.


# ==========================================================
# ========== Paramètres de la course =======================
# ==========================================================

LONGEUR_COURSE = 100
# Nombre de colonnes à parcourir pour atteindre l’arrivée.
# Plus ce nombre est grand, plus la course dure longtemps.

NB_PROCESS = 20
# Nombre de chevaux / processus.
# Ici, on aura 20 chevaux qui courent en parallèle.


# Chaque cheval sera dessiné sur plusieurs lignes de texte ASCII.
HORSE_ART = [
    r"  ,^v^,",
    r" ( A  )>",
    r"  / \  "
]
# Représentation visuelle du cheval.
# Le "A" sera remplacé par la lettre du cheval :
# A, B, C, D, etc.

HORSE_H = len(HORSE_ART)
# Hauteur du cheval en nombre de lignes.
# Ici HORSE_ART contient 3 lignes, donc HORSE_H = 3.

LANE_H = HORSE_H + 1
# Hauteur d'une "voie" de course.
# On ajoute 1 ligne vide entre deux chevaux
# pour que l’affichage soit plus lisible.

lyst_colors = [CL_WHITE, CL_RED, CL_GREEN, CL_BROWN, CL_BLUE, CL_MAGENTA, CL_CYAN, CL_GRAY]
# Liste des couleurs disponibles pour colorer les chevaux.
# Si on a plus de chevaux que de couleurs, on recommence depuis le début
# grâce à l'opérateur modulo plus bas.


# ==========================================================
# ========== Variables partagées entre processus ===========
# ==========================================================

keep_running = mp.Value(ctypes.c_bool, True)
# Variable partagée booléenne.
# Elle vaut True tant que la course continue.
# Quand un cheval gagne, on la met à False pour stopper les autres.
#
# mp.Value(...) crée une valeur partagée entre plusieurs processus.
# ctypes.c_bool indique que le type stocké est un booléen de type C.

positions = mp.Array('i', [1] * NB_PROCESS)
# Tableau partagé d'entiers.
# positions[i] contient la colonne actuelle du cheval i.
#
# 'i' signifie "integer" (entier).
# [1] * NB_PROCESS crée une liste initiale remplie de 1 :
# tous les chevaux commencent en colonne 1.

winner = mp.Value('i', -1)
# Variable partagée pour stocker le gagnant.
# Elle vaut :
# -1  -> aucun gagnant pour l’instant
# 0   -> cheval A
# 1   -> cheval B
# etc.

screen_lock = mp.Lock()
# Verrou (mutex) pour protéger l'écran.
# Sans ce verrou, plusieurs processus pourraient écrire en même temps
# dans le terminal, ce qui mélangerait complètement l'affichage.

winner_lock = mp.Lock()
# Verrou pour protéger l’accès à la variable "winner".
# Il empêche deux chevaux de se déclarer gagnants en même temps.


def horse_base_line(i: int) -> int:
    """
    Renvoie la ligne de départ (en indexation 1) du cheval i.
    Chaque cheval est affiché sur plusieurs lignes.
    """
    return 1 + i * LANE_H
    # Exemple :
    # cheval 0 -> ligne 1
    # cheval 1 -> ligne 1 + LANE_H
    # cheval 2 -> ligne 1 + 2*LANE_H
    # etc.
    #
    # Cela permet d’attribuer une "voie" différente à chaque cheval.


def draw_horse(i: int, col: int):
    """
    Affiche le cheval i à la colonne col.
    Le cheval occupe HORSE_H lignes.
    """
    base = horse_base_line(i)
    # Ligne de départ de la voie du cheval i.

    color = lyst_colors[i % len(lyst_colors)]
    # Choisit une couleur pour ce cheval.
    # Le modulo permet de boucler si on a plus de chevaux que de couleurs.

    letter = chr(ord('A') + i)
    # Transforme l'indice i en lettre :
    # 0 -> A
    # 1 -> B
    # 2 -> C
    # etc.

    en_couleur(color)
    # On active la couleur du cheval avant de l'afficher.

    for k, line in enumerate(HORSE_ART):
        # On parcourt les 3 lignes du dessin ASCII du cheval.
        # k = 0, 1, 2
        # line = contenu de chaque ligne

        move_to(base + k, 1)
        # On se place au début de la ligne à redessiner.

        erase_line_from_beg_to_curs()
        # On efface le début de la ligne pour enlever l'ancien dessin.

        move_to(base + k, col)
        # On se place à la bonne colonne pour dessiner le cheval à sa nouvelle position.

        # Remplace le 'A' dans le dessin par la lettre du cheval
        # pour qu'on puisse l'identifier visuellement.
        print(line.replace("A", letter), end='', flush=True)

    print(NORMAL, end='', flush=True)
    # On remet les couleurs du terminal à la normale après affichage.


def un_cheval(i: int):
    """
    Fonction exécutée par un processus-cheval.
    Chaque cheval avance de façon aléatoire jusqu'à la ligne d'arrivée
    ou jusqu'à ce qu'un autre ait gagné.
    """
    col = 1
    # Position de départ du cheval : colonne 1.

    while keep_running.value and col < LONGEUR_COURSE:
        # Tant que :
        # - la course n'est pas arrêtée
        # - le cheval n'a pas encore atteint l’arrivée
        # alors il continue à courir.

        col += 1
        # Le cheval avance d'une colonne à chaque boucle.
        # Ici, tous avancent d'un pas fixe.
        # La différence de vitesse vient surtout du sleep aléatoire.

        positions[i] = col
        # On met à jour la position du cheval dans le tableau partagé.
        # Cela permettra à l'arbitre de savoir qui est devant et derrière.

        # Affichage protégé : une seule écriture écran à la fois
        with screen_lock:
            draw_horse(i, col)
        # with lock: signifie :
        # - on prend le verrou
        # - on exécute le bloc
        # - on libère le verrou automatiquement
        #
        # Cela évite que deux chevaux dessinent en même temps.

        time.sleep(0.05 * random.randint(1, 6))
        # Pause aléatoire pour simuler des vitesses différentes.
        # random.randint(1, 6) donne un entier entre 1 et 6.
        #
        # Donc le temps de pause vaut :
        # 0.05, 0.10, 0.15, ..., 0.30 seconde
        #
        # Un cheval qui dort moins avance plus vite.

    # Si on sort de la boucle, deux cas possibles :
    # 1) la course a été stoppée car un autre cheval a gagné
    # 2) ce cheval a lui-même atteint la ligne d'arrivée

    if col >= LONGEUR_COURSE:
        # Si le cheval a atteint ou dépassé la ligne d’arrivée,
        # il tente de devenir le gagnant officiel.

        with winner_lock:
            # On protège cette section critique :
            # plusieurs chevaux peuvent finir presque en même temps.
            if winner.value == -1:
                # Si personne n’a encore été déclaré gagnant,
                # alors ce cheval devient le gagnant.
                winner.value = i

                keep_running.value = False
                # On arrête la course pour les autres chevaux.


def arbitre():
    """
    Processus arbitre :
    il surveille en permanence les positions des chevaux
    et affiche le premier et le dernier.
    """
    info_line = 1 + NB_PROCESS * LANE_H + 1
    # Ligne sous toutes les voies de course.
    # C’est là qu’on affichera les infos de l’arbitre.

    while keep_running.value:
        # Tant que la course n’est pas finie, l’arbitre observe.

        pos = list(positions)
        # On prend un "snapshot" des positions actuelles.
        # list(positions) copie les valeurs du tableau partagé
        # dans une liste Python normale.
        #
        # Cela évite d’avoir des lectures qui changent au milieu du traitement.

        leader = max(range(NB_PROCESS), key=lambda k: pos[k])
        # On cherche l’indice du cheval le plus avancé.
        # range(NB_PROCESS) donne 0,1,2,...,NB_PROCESS-1
        # key=lambda k: pos[k] signifie qu’on compare selon la position du cheval k.

        last = min(range(NB_PROCESS), key=lambda k: pos[k])
        # Même idée, mais pour trouver le dernier.

        with screen_lock:
            # On verrouille l'écran pour que l'arbitre n'écrive pas
            # en même temps qu'un cheval.

            move_to(info_line, 1)
            # On place le curseur sur la ligne d’information.

            erase_line_from_beg_to_curs()
            # On efface la ligne précédente.

            print(
                f"Arbitre: 1er = {chr(ord('A')+leader)} (col {pos[leader]}) | "
                f"dernier = {chr(ord('A')+last)} (col {pos[last]})",
                end='',
                flush=True
            )
            # Affiche le leader et le dernier avec leur lettre et leur colonne.

        time.sleep(0.1)
        # L’arbitre met à jour l’affichage toutes les 0.1 seconde.


def demander_pari() -> int:
    """
    Demande à l'utilisateur sur quel cheval il parie.
    Retourne l'indice du cheval choisi :
    A -> 0, B -> 1, C -> 2, etc.
    """
    while True:
        rep = input(f"Parie sur un cheval (A-{chr(ord('A')+NB_PROCESS-1)}) : ").strip().upper()
        # input(...) lit ce que tape l’utilisateur.
        # .strip() enlève les espaces au début et à la fin.
        # .upper() convertit en majuscule.

        if len(rep) == 1:
            # On accepte seulement une seule lettre.

            idx = ord(rep) - ord('A')
            # Convertit une lettre en indice :
            # A -> 0
            # B -> 1
            # C -> 2
            # etc.

            if 0 <= idx < NB_PROCESS:
                # On vérifie que l’indice correspond bien à un cheval existant.
                return idx

        print("Entrée invalide. Exemple: A")
        # Si ce n’est pas valide, on redemande.


def course_hippique():
    """
    Fonction principale :
    - demande le pari de l’utilisateur
    - prépare l’écran
    - lance les processus chevaux
    - lance l’arbitre
    - attend la fin de la course
    - affiche le résultat final
    """
    # Pari AVANT d’effacer l’écran.
    # Sinon l’utilisateur ne verrait pas ce qu’il tape.
    bet = demander_pari()

    effacer_ecran()
    # Efface le terminal pour préparer l'affichage dynamique.

    curseur_invisible()
    # Cache le curseur pour un rendu plus propre.

    # ------------------------------------------------------
    # Création des processus "chevaux"
    # ------------------------------------------------------
    chevaux = []
    # Liste qui contiendra les objets Process des chevaux.

    for i in range(NB_PROCESS):
        p = Process(target=un_cheval, args=(i,))
        # Crée un processus qui exécutera un_cheval(i).
        #
        # target=un_cheval : fonction à exécuter
        # args=(i,)        : argument passé à la fonction
        # Attention : (i,) avec la virgule = tuple à un seul élément

        p.start()
        # Lance réellement le processus.

        chevaux.append(p)
        # On garde une référence vers ce processus
        # pour pouvoir ensuite faire p.join().

    # ------------------------------------------------------
    # Création du processus arbitre
    # ------------------------------------------------------
    p_arbitre = Process(target=arbitre)
    # Crée un processus séparé pour l’arbitre.

    p_arbitre.start()
    # Lance l’arbitre.

    # ------------------------------------------------------
    # Attente de la fin des chevaux
    # ------------------------------------------------------
    for p in chevaux:
        p.join()
        # join() bloque le processus principal
        # jusqu’à la fin du processus p.
        #
        # Donc ici, on attend que tous les chevaux aient terminé.

    keep_running.value = False
    # Par sécurité, on arrête aussi l’arbitre.

    p_arbitre.join(timeout=1.0)
    # On attend que l’arbitre se termine.
    # timeout=1.0 signifie qu’on attend au maximum 1 seconde.

    # ------------------------------------------------------
    # Affichage du résultat final
    # ------------------------------------------------------
    info_line = 1 + NB_PROCESS * LANE_H + 3
    # Ligne plus basse que celle de l’arbitre,
    # pour afficher le message final.

    with screen_lock:
        # On protège aussi l’affichage final.

        move_to(info_line, 1)
        # On place le curseur sur la ligne d’affichage du résultat.

        print(NORMAL, end='', flush=True)
        # On remet la couleur par défaut.

        if winner.value != -1:
            # Si un gagnant a bien été enregistré :

            w = winner.value
            # Indice du gagnant

            w_letter = chr(ord('A') + w)
            # Conversion de l’indice en lettre

            bet_letter = chr(ord('A') + bet)
            # Lettre du cheval parié par l’utilisateur

            print(
                f"GAGNANT = {w_letter} | ton pari = {bet_letter} => "
                f"{'✅ Gagné !' if w == bet else '❌ Perdu.'}",
                flush=True
            )
            # Affiche le gagnant et indique si le pari est bon.

        else:
            # Cas de secours : aucun gagnant détecté
            print("Fin (aucun gagnant détecté)", flush=True)

    curseur_visible()
    # Très important : on remet le curseur visible à la fin.
    # Sinon le terminal peut rester "bizarre" visuellement.


if __name__ == "__main__":
    # Ce bloc signifie :
    # "exécuter course_hippique() seulement si ce fichier
    # est lancé directement"
    #
    # C’est indispensable avec multiprocessing,
    # surtout sous Windows, sinon les processus fils
    # peuvent relancer tout le programme.
    course_hippique()