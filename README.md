//// FONCTIONNEMENT PRINCIPAL DU KERNEL ////

1. Lors du Makefile,
   - boot/boot.asm est assemble avec nasm
   - le noyau C (srcs/kernel/*.c) est compile avec gcc -m32 en freestanding
   - ld relie le tout avec linker.ld pour produire kernel.bin

2. GRUB charge kernel.bin.
   - grub.cfg dit a GRUB quel kernel lancer
   - GRUB trouve le header multiboot dans boot.asm
   - puis GRUB saute au point d'entree start

3. Le code ASM prepare le kernel.
   - il initialise la stack
   - il recupere les infos donnees par GRUB
   - il appelle kernel_main en C

4. Le code C prend la main.
   - kernel_main est le debut du kernel
   - pour l'instant il ecrit directement dans la memoire VGA
   - il affiche "42" a l'ecran

5. Le kernel reste dans une boucle infinie.



//// STRUCTURE DU KERNEL ////

--- boot/boot.asm ---
C'est le tout premier code execute quand le processeur demarre.
• Le "header multiboot" en haut : des constantes (MAGIC, FLAGS, CHECKSUM) que GRUB lit pour reconnaitre que ce binaire est un kernel valide et savoir comment le charger.
• La stack : une zone de 16 Ko reservee en memoire ; le kernel en a besoin pour appeler des fonctions.
• start : le point d'entree. On met la stack en place (esp), on pousse les infos fournies par GRUB (eax, ebx), puis on appelle kernel_main (la partie C). Ensuite on boucle sans fin (cli/hlt/jmp) pour ne pas quitter le kernel.

- Multiboot header :
  - MBALIGN et MEMINFO sont des flags pour que GRUB sache comment charger le kernel
  - equ est une directive pour definir une constante
  - Magic number : 0x1BADB002 (il est utilise pour que GRUB reconnaisse le kernel comme valide)
  - Flags : 1 << 0 (MBALIGN) | 1 << 1 (MEMINFO)
  - Checksum : -(MAGIC + FLAGS) (le checksum sert a )

- section .multiboot :
  - align 4 : aligne le header sur 4 octets
  - dd va definir les valeurs des constantes MAGIC, FLAGS et CHECKSUM

- Stack :
  - 16 KB de stack
  - resb 16384 : reserve 16384 octets pour la stack
  - stack_bottom: est le debut de la stack
  - stack_top: est la fin de la stack

- Code du kernel :
  - mov esp, stack_top : initialise la stack
  - pousse ebx et eax sur la stack
  - call kernel_main : appelle la fonction kernel_main

  - .hang est une etiquette pour la boucle infinie
  - cli : desactive les interruptions
  - hlt : met le processeur en pause
  - jmp .hang : reboucle sur la boucle infinie





--- grub/grub.cfg ---
Configuration du chargeur de demarrage GRUB. Au boot, c'est ce fichier que GRUB lit
• timeout=0 : pas d'attente au menu, demarrage direct.
• default=0 : premiere entree du menu choisie par defaut.
• menuentry "kfs" : une entree nommee "kfs" qui charge le kernel avec multiboot /boot/kernel.bin puis lance boot. Le chemin /boot/kernel.bin est celui vu par GRUB sur l'image (iso).




--- linker.ld ---
Script pour le linker (ld). Il dit comment assembler les .o (dossier `build/`) et le code C en un seul fichier kernel.bin.
• ENTRY(start) : le point d'entree du programme est la symbole start (defini dans boot.asm).
• . = 1M : le kernel est place a 1 Mo en memoire (convention multiboot).

Sections et ALIGN :
• .multiboot ALIGN(4) : contient le header multiboot (MAGIC, FLAGS, CHECKSUM) produit par boot.asm. Doit etre au tout debut du binaire pour que GRUB le trouve. ALIGN(4) = la section commence a une adresse multiple de 4 octets (obligatoire pour le format multiboot).
• .text ALIGN(4K) : le code executable (instructions du kernel, ASM + C). ALIGN(4K) = alignement sur 4096 octets (une page) ; le processeur gere mieux le code aligne sur des pages.
• .rodata ALIGN(4K) : donnees en lecture seule (constantes, chaines, etc.). Separees du code pour pouvoir les proteger en lecture seule en memoire. ALIGN(4K) pour etre sur une page.
• .data ALIGN(4K) : donnees initialisees modifiables (variables globales avec valeur de depart). ALIGN(4K) pour etre sur une page.
• .bss ALIGN(4K) : donnees non initialisees (variables globales a zero : stack du kernel, buffers, etc.). Ne prend pas de place dans le fichier, mais en memoire au chargement. ALIGN(4K) pour etre sur une page.

En resume : ALIGN(n) force le debut de la section a une adresse multiple de n (4 = 4 octets, 4K = 4096 octets). Ca evite des acces mal alignes et ameliore les performances.

• /DISCARD/ : on jette les sections inutiles (eh_frame, comment, etc.) pour garder le binaire petit.





--- srcs/include/vga.h ---
Declarations pour l'ecran texte VGA (memoire 0xB8000) : structure d'etat et fonctions vga_init, vga_clear_screen, vga_print_string.

--- srcs/kernel/kernel.c ---
Point d'entree du noyau en C : kernel_main(magic, info) appelee depuis boot.asm avec les parametres multiboot. Initialise le VGA et affiche un message, puis boucle infiniment.

--- srcs/kernel/vga.c ---
Implementation de l'affichage VGA (caracteres, retour ligne, defilement).


--- Makefile ---
Automatise la construction du projet.
• Assemble boot.asm → build/boot.o (nasm).
• Compile les fichiers C sous srcs/kernel/ vers build/*.o avec gcc -m32 -ffreestanding -nostdlib et -Isrcs/include (flags du projet).
• Lie build/boot.o + les .o C avec linker.ld → kernel.bin (ld).
• Copie kernel.bin et grub.cfg dans iso/boot/, puis fabrique l'ISO avec grub-mkrescue.
• make run : lance QEMU avec l'ISO pour tester le kernel.

--------------------- Kernel --------------------------

```c
__asm__ (
    "code assembleur"
    : outputs
    : inputs
    : clobbers
);

__asm__("pause" ::: "memory");
             ↑   ↑   ↑
         out   in   clobber
```

| Élément   | Rôle précis |
|----------|------------|
| outputs  | Variables C dans lesquelles l’ASM écrit des résultats (le compilateur récupère ces valeurs après l’exécution) |
| inputs   | Valeurs C que le compilateur charge dans des registres ou en mémoire pour que l’ASM les utilise |
| clobbers | Ressources que l’ASM modifie sans passer par outputs (registres, flags, mémoire), et que le compilateur doit considérer comme invalidées |


```asm
pause   ; attente optimisée
hlt     ; stop CPU
nop     ; no operation
cli     ; disable interrupts
sti     ; enable interrupts
```

```asm
| Clobber    | Signification                 |
| ---------- | ----------------------------- |
| `"memory"` | toute la mémoire peut changer |
| `"cc"`     | flags CPU modifiés            |

```

```asm
"eax", "ebx", "ecx", "edx",
"esi", "edi", "ebp", "esp"
```


| Type          | Exemple  | Mémoire  |
| ------------- | -------- | -------- |
| Text mode     | 80×25    | 0xB8000  |
| Graphics mode | 320×200  | 0xA0000  |
| VBE           | 1024×768 | variable |

[ attribut ][ caractère ]
   1 byte      1 byte


[ attribut (8 bits) ][ caractère ASCII (8 bits) ]

[ H ][ e ][ l ][ l ][ o ]
[ attr,H ][ attr,e ][ attr,l ][ attr,l ][ attr,o ]

Mémoire (0xB8000)
        ↓
caractère ASCII ('A')
        ↓
lookup font VGA
        ↓
bitmap pixels
        ↓
affichage écran


bit:   7     6   5   4     3   2   1   0
       |     |   |   |     |   |   |   |
       |     └───────┘     └───────────┴── foreground (texte)
       |           background
       |
       └── blink / high background (selon config)


| Couleur     | Code |
| ----------- | ---- |
| Noir        | 0    |
| Bleu        | 1    |
| Vert        | 2    |
| Cyan        | 3    |
| Rouge       | 4    |
| Magenta     | 5    |
| Marron      | 6    |
| Gris clair  | 7    |
| Gris foncé  | 8    |
| Bleu clair  | 9    |
| Vert clair  | 10   |
| Cyan clair  | 11   |
| Rouge clair | 12   |
| Rose        | 13   |
| Jaune       | 14   |
| Blanc       | 15   |

------DEBUG----------

(qemu) info registers
(qemu) info cpus
(qemu) xp /16x 0x100000
(qemu) xp /32bx 0xb8000
(qemu) xp /32cb 0xb8000
(qemu) xp /80cb 0xb8640
(qemu) xp /16bx 0xb8640

(qemu) xp /10x 0x1000
0000000000001000: 0x0905c689 0xff000000 0x48868de0 0x89000000
0000000000001010: 0x00004086 0xb0868d00 0x89000000 0x00003286
0000000000001020: 0x96010f00 0x00000030
(qemu) stop
(qemu) cont

(qemu) system_reset
```bash
qemu-system-i386 -cdrom kfs.iso -s -S
🔹 options :
-S → CPU bloqué au démarrage
-s → ouvre un serveur GDB sur localhost:1234
```

```bash
gdb
(gdb) target remote localhost:1234


(gdb) break kernel_main
(gdb) continue
```