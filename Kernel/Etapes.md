# Kernel Rootkit - Étapes de développement

## Vue d'ensemble des niveaux

| Niveau | Technique | Complexité | État |
|--------|-----------|------------|------|
| **1** | Userland Hooking (LD_PRELOAD) | ⭐ | ✅ Complété |
| **2** | Kernel Module (LKM) simple | ⭐⭐ | 🔄 En cours |
| **3** | DKOM (Direct Kernel Object Manipulation) | ⭐⭐⭐ | 🔲 À faire |
| **4** | eBPF Interception | ⭐⭐⭐⭐ | 🔲 À faire |

---

## Niveau 2 : Kernel Module (LKM)

### Étape 2.1 : Premier module kernel ✅
- [x] Créer un module kernel minimal
- [x] Implémenter `module_init()` et `module_exit()`
- [x] Utiliser `printk()` pour les logs
- [x] Compiler avec le Makefile kernel
- [x] Tester avec `insmod`, `rmmod`, `dmesg`

### Étape 2.2 : Hook de syscall (`getdents64`) 🔄

#### 2.2.1 : Préparation des headers et variables
- [ ] Ajouter les headers nécessaires :
  - `<linux/syscalls.h>` : définitions des syscalls
  - `<linux/dirent.h>` : structure `linux_dirent64`
  - `<linux/uaccess.h>` : accès mémoire user space
  - `<linux/kallsyms.h>` : pour trouver `sys_call_table`
  - `<asm/paravirt.h>` : pour manipuler CR0
- [ ] Définir le typedef pour la syscall originale :
  ```c
  typedef asmlinkage long (*original_getdents64_t)(
      unsigned int fd,
      struct linux_dirent64 __user *dirent,
      unsigned int count);
  ```
- [ ] Déclarer les variables globales :
  - `static original_getdents64_t original_getdents64;`
  - `static unsigned long *sys_call_table;`

#### 2.2.2 : Manipulation de CR0
- [ ] Créer une fonction pour désactiver Write Protect :
  - Lire le registre CR0
  - Désactiver le bit WP (bit 16)
  - Écrire dans CR0
- [ ] Créer une fonction pour réactiver Write Protect :
  - Réactiver le bit WP
  - Écrire dans CR0

#### 2.2.3 : Trouver `sys_call_table`
- [ ] **Kernel < 5.7** : Utiliser `kallsyms_lookup_name("sys_call_table")`
- [ ] **Kernel ≥ 5.7** : Utiliser la technique kprobes :
  ```c
  #include <linux/kprobes.h>
  static struct kprobe kp = {
      .symbol_name = "kallsyms_lookup_name"
  };
  ```
- [ ] Valider que l'adresse trouvée n'est pas NULL

#### 2.2.4 : Installer le hook
- [ ] Dans `rootkit_init()` :
  1. Trouver l'adresse de `sys_call_table`
  2. Sauvegarder le pointeur original de `getdents64`
  3. Désactiver Write Protect (CR0)
  4. Remplacer l'entrée par notre fonction hook
  5. Réactiver Write Protect

#### 2.2.5 : Désinstaller le hook
- [ ] Dans `rootkit_exit()` :
  1. Désactiver Write Protect
  2. Restaurer le pointeur original
  3. Réactiver Write Protect

### Étape 2.3 : Créer la fonction hook
- [ ] Créer `hooked_getdents64()` avec la même signature
- [ ] Appeler la fonction originale
- [ ] Logger les appels pour validation
- [ ] Retourner le résultat de l'original (transparent pour l'instant)

### Étape 2.4 : Filtrage des PIDs
- [ ] Parser le buffer retourné par `getdents64`
- [ ] Comprendre la structure `linux_dirent64` :
  ```c
  struct linux_dirent64 {
      u64        d_ino;      // Inode number
      s64        d_off;      // Offset to next entry
      unsigned short d_reclen; // Length of this entry
      unsigned char  d_type;   // File type
      char       d_name[];    // Filename (null-terminated)
  };
  ```
- [ ] Identifier les entrées correspondant au PID cible
- [ ] Modifier le buffer pour "sauter" ces entrées
- [ ] Ajuster la valeur de retour (taille du buffer)

### Étape 2.5 : Configuration dynamique du PID
- [ ] Option A : Paramètre de module (`module_param`)
- [ ] Option B : Fichier dans `/proc` (procfs)
- [ ] Option C : Variable globale modifiable

### Étape 2.6 : Tests et validation
- [ ] Tester avec `ls /proc`
- [ ] Vérifier que le PID cible est invisible
- [ ] Tester avec `ps aux`
- [ ] Vérifier l'absence de kernel panic
- [ ] Valider le déchargement propre du module

---

## Niveau 3 : DKOM (Direct Kernel Object Manipulation)

### Étape 3.1 : Comprendre les structures kernel
- [ ] Étudier la structure `module` (`<linux/module.h>`)
- [ ] Comprendre la liste chaînée des modules (`THIS_MODULE`)
- [ ] Identifier les champs `list` et `mkobj.kobj`

### Étape 3.2 : Cacher le module de `lsmod`
- [ ] Retirer le module de la liste des modules :
  ```c
  list_del(&THIS_MODULE->list);
  ```
- [ ] Le module reste chargé mais invisible dans `lsmod`

### Étape 3.3 : Cacher de `/sys/module/`
- [ ] Retirer l'entrée kobject :
  ```c
  kobject_del(&THIS_MODULE->mkobj.kobj);
  ```
- [ ] Le module devient invisible dans `/sys/module/`

### Étape 3.4 : Cacher de `/proc/modules`
- [ ] Cette liste utilise la même structure que `lsmod`
- [ ] `list_del` devrait suffire

### Étape 3.5 : Gestion de la suppression
- [ ] Problème : Comment décharger un module invisible ?
- [ ] Solution : Garder une référence ou trigger externe
- [ ] Implémenter un mécanisme de "révélation" (signal, fichier, etc.)

---

## Niveau 4 : eBPF Interception

### Étape 4.1 : Environnement eBPF
- [ ] Installer les outils : `libbpf-dev`, `clang`, `llvm`, `linux-headers`
- [ ] Comprendre l'architecture eBPF (kernel space vs user space)
- [ ] Se familiariser avec les types de programmes eBPF

### Étape 4.2 : Premier programme eBPF
- [ ] Créer un programme eBPF minimal
- [ ] Écrire le code kernel space (`.bpf.c`)
- [ ] Écrire le loader user space (`.c`)
- [ ] Compiler et charger

### Étape 4.3 : Hook sur `getdents64`
- [ ] S'attacher à la syscall avec un tracepoint ou kprobe
- [ ] Intercepter les appels
- [ ] Logger pour validation

### Étape 4.4 : eBPF Maps
- [ ] Créer une BPF map pour stocker le PID à cacher
- [ ] Permettre la mise à jour depuis user space
- [ ] Lire la map dans le programme kernel

### Étape 4.5 : Filtrage
- [ ] Implémenter la logique de comparaison
- [ ] Modifier le buffer de retour (si possible avec eBPF)
- [ ] Gérer les limitations du vérifieur eBPF

### Étape 4.6 : Programme de contrôle
- [ ] Créer un CLI pour :
  - Charger/décharger le programme eBPF
  - Définir le PID à cacher
  - Afficher l'état

---

## Notes techniques

### Numéros de syscall (x86_64)
- `getdents` : 78
- `getdents64` : 217

### Vérifier la version du kernel
```bash
uname -r
```

### Trouver l'adresse de sys_call_table
```bash
sudo cat /proc/kallsyms | grep sys_call_table
```

### Logs kernel
```bash
sudo dmesg | tail -20
sudo dmesg -w  # Watch mode
```

---

## Ressources

- [Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
- [Syscall Table x86_64](https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/)
- [eBPF Documentation](https://ebpf.io/what-is-ebpf/)
- [libbpf Documentation](https://libbpf.readthedocs.io/)
