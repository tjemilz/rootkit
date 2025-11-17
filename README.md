### Disclaimer ### 

Ce projet est uniquement à but éducatif. En aucun cas, il ne saurait être utilisé pour effectuer des actions malveillantes sur un système que vous ne possédez pas. 


### Descriptif ### 

Ce projet a pour but de créér un rootkit minimaliste. Voici les différentes étapes : 
- [ ]  **Comprendre la structure d'un fichier `.so` :** Réussir à compiler un simple fichier C en une bibliothèque partagée (`.so`).
- [ ]  **Comprendre `LD_PRELOAD` :** Écrire une fonction `main` simple dans un exécutable C, puis écrire une petite fonction dans le `.so` qui est exécutée **avant** le `main` grâce à l'attribut `__attribute__((constructor))`. Valider le chargement.
- [ ]  **Gestion des Symboles :** Utiliser la fonction **`dlsym`** avec l'argument spécial **`RTLD_NEXT`** pour obtenir le pointeur de la **vraie** fonction de la librairie standard (par exemple, la vraie `puts` de la `libc`).
- [ ]  **Définir la Fonction Cible :** Identifier la fonction standard à intercepter pour filtrer le contenu de `/proc`. La fonction **`readdir`** (qui lit les entrées d'un répertoire) est la plus pertinente pour cacher les répertoires PID.
- [ ]  **Créer la Fonction Surchargée (le Hook) :** Écrire votre propre version de `readdir` qui a exactement la même signature. Elle doit appeler l'**original** (le vrai `readdir` obtenu en 1.3) pour obtenir une entrée.
- [ ]  **Tester le Filtrage Statique :** Implémenter la logique dans votre `readdir` surchargée pour vérifier si l'entrée de répertoire est un **PID statique prédéfini** (ex: `5000`). Si c'est le cas, ignorer cette entrée en bouclant et en appelant à nouveau l'original.
- [ ]  **Test de Preuve de Concept :** Charger votre `.so` avec `LD_PRELOAD` sur un outil simple de liste de répertoires (`ls /proc`). Valider que le répertoire du PID statique (ex: `/proc/5000`) est bien invisible.
- [ ]  **Récupération du PID à Cacher :** Modifier la fonction `constructor` (étape 1.2) pour récupérer le PID du processus à masquer. Une technique simple et robuste est de lire ce PID depuis une **variable d'environnement** (ex: `HIDE_ME_PID`)
- [ ]  **Filtrage Dynamique :** Modifier la logique de la fonction `readdir` surchargée pour comparer l'entrée lue (`dirp->d_name`) avec le PID **dynamique** récupéré en 3.1.
- [ ]  **Test du Rootkit Complet :** Créer un petit programme "lanceur" en C (ou un script shell) qui : 1) Lance le processus **cible** (celui que vous voulez cacher) en arrière-plan et note son PID. 2) Définit la variable `HIDE_ME_PID` avec ce PID. 3) Exécute l'outil de surveillance (`ps`, `top`) avec `LD_PRELOAD` pointant vers votre `.so`. Valider que le PID n'apparaît pas.
- [ ]  **Gestion des Autorisations :** Ajouter une vérification simple pour que votre hook ne s'active que si un certain "mot de passe" ou une variable secrète est présente dans l'environnement. Cela empêche le hook de s'activer par erreur sur tous les programmes.
- [ ]  **Nettoyage de l'Environnement :** Après avoir lu le PID à cacher, utiliser **`unsetenv`** pour supprimer la variable d'environnement (`HIDE_ME_PID`) de la mémoire du processus. Cela ajoute une couche de furtivité pour qu'un attaquant ne puisse pas trouver le PID caché en examinant les variables d'environnement.


### Résolution ### 

To test the function : 
- Compile the program using make
- Using a terminal : (to hide PID number 5000)
ROOTKIT_PWD=password123 HIDE_ME_PID=5000 LD_PRELOAD=./hook.so ls /proc