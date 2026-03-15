//// FONCTIONNEMENT PRINCIPAL DU KERNEL ////

1. Lors du Makefile,
   - boot/boot.asm est assemble avec nasm
   - le code Rust est compile avec cargo
   - ld relie le tout avec linker.ld pour produire kernel.bin

2. GRUB charge kernel.bin.
   - grub.cfg dit a GRUB quel kernel lancer
   - GRUB trouve le header multiboot dans boot.asm
   - puis GRUB saute au point d'entree start

3. Le code ASM prepare le kernel.
   - il initialise la stack
   - il recupere les infos donnees par GRUB
   - il appelle kernel_main en Rust

4. Le code Rust prend la main.
   - kernel_main est le debut du kernel
   - pour l'instant il ecrit directement dans la memoire VGA
   - il affiche "42" a l'ecran

5. Le kernel reste dans une boucle infinie.



//// STRUCTURE DU KERNEL ////

--- boot/boot.asm ---
C’est le tout premier code exécuté quand le processeur démarre.
• Le "header multiboot" en haut : des constantes (MAGIC, FLAGS, CHECKSUM) que GRUB lit pour reconnaître que ce binaire est un kernel valide et savoir comment le charger.
• La stack : une zone de 16 Ko réservée en mémoire ; le kernel en a besoin pour appeler des fonctions.
• start : le point d’entrée. On met la stack en place (esp), on pousse les infos fournies par GRUB (eax, ebx), puis on appelle kernel_main (la partie Rust). Ensuite on boucle sans fin (cli/hlt/jmp) pour ne pas quitter le kernel.

- Multiboot header : 
  - MBALIGN et MEMINFO sont des flags pour que GRUB sache comment charger le kernel
  - equ est une directive pour definir une constante
  - Magic number : 0x1BADB002 (il est utilisé pour que GRUB reconnaisse le kernel comme valide)
  - Flags : 1 << 0 (MBALIGN) | 1 << 1 (MEMINFO) 
  - Checksum : -(MAGIC + FLAGS) (le checksum sert à )

- section .multiboot :
  - align 4 : aligne le header sur 4 octets
  - dd va définir les valeurs des constantes MAGIC, FLAGS et CHECKSUM

- Stack :
  - 16 KB de stack
  - resb 16384 : reserve 16384 octets pour la stack
  - stack_bottom: est le debut de la stack
  - stack_top: est la fin de la stack

- Code du kernel :
  - mov esp, stack_top : initialise la stack
  - pousse ebx et eax sur la stack
  - call kernel_main : appelle la fonction kernel_main

  - .hang est une étiquette pour la boucle infinie
  - cli : désactive les interruptions
  - hlt : met le processeur en pause
  - jmp .hang : reboucle sur la boucle infinie





--- grub/grub.cfg ---
Configuration du chargeur de démarrage GRUB. Au boot, c’est ce fichier que GRUB lit
• timeout=0 : pas d’attente au menu, démarrage direct.
• default=0 : première entrée du menu choisie par défaut.
• menuentry "kfs" : une entrée nommée "kfs" qui charge le kernel avec multiboot /boot/kernel.bin puis lance boot. Le chemin /boot/kernel.bin est celui vu par GRUB sur l’image (iso).




--- linker.ld ---
Script pour le linker (ld). Il dit comment assembler boot.o et le code Rust en un seul fichier kernel.bin.
• ENTRY(start) : le point d’entrée du programme est la symbole start (défini dans boot.asm).
• . = 1M : le kernel est placé à 1 Mo en mémoire (convention multiboot).

Sections et ALIGN :
• .multiboot ALIGN(4) : contient le header multiboot (MAGIC, FLAGS, CHECKSUM) produit par boot.asm. Doit être au tout début du binaire pour que GRUB le trouve. ALIGN(4) = la section commence à une adresse multiple de 4 octets (obligatoire pour le format multiboot).
• .text ALIGN(4K) : le code exécutable (instructions du kernel, ASM + Rust). ALIGN(4K) = alignement sur 4096 octets (une page) ; le processeur gère mieux le code aligné sur des pages.
• .rodata ALIGN(4K) : données en lecture seule (constantes, chaînes, etc.). Séparées du code pour pouvoir les protéger en lecture seule en mémoire. ALIGN(4K) pour être sur une page.
• .data ALIGN(4K) : données initialisées modifiables (variables globales avec valeur de départ). ALIGN(4K) pour être sur une page.
• .bss ALIGN(4K) : données non initialisées (variables globales à zéro : stack du kernel, buffers, etc.). Ne prend pas de place dans le fichier, mais en mémoire au chargement. ALIGN(4K) pour être sur une page.

En résumé : ALIGN(n) force le début de la section à une adresse multiple de n (4 = 4 octets, 4K = 4096 octets). Ça évite des accès mal alignés et améliore les performances.

• /DISCARD/ : on jette les sections inutiles (eh_frame, comment, etc.) pour garder le binaire petit.




--- src/lib.rs ---
Point d’entrée du crate Rust (pas de main, c’est un kernel).
• #![no_std] : pas de bibliothèque standard (il n’y a pas d’OS sous le kernel).
• #![no_main] : pas de fonction main ; l’entrée réelle est kernel_main, appelée depuis l’ASM.
• mod kernel : déclare le module kernel (fichier kernel.rs).
• panic_handler : en cas de panic, on boucle indéfiniment (pas d’OS pour afficher un message). Obligatoire en no_std.

--- src/kernel.rs ---
C’est ici que la logique du kernel en Rust vit. À compléter par la personne qui gère le Rust.
• kernel_main(magic, info) : appelée par boot.asm avec les paramètres multiboot (magic number et pointeur vers les infos GRUB). La signature doit rester exactement comme ça pour que l’appel depuis l’ASM fonctionne.
• #[no_mangle] et extern "C" : pour que le nom de la fonction ne soit pas modifié et que l’appel depuis l’ASM en C soit valide.
• Le corps doit implémenter l’interface écran et afficher "42", puis terminer dans une boucle infinie (le kernel ne doit jamais retourner).

--- Cargo.toml ---
Configuration du projet Rust.
• crate-type = ["staticlib"] : on produit une bibliothèque statique (.a), pas un exécutable ; le Makefile link cette lib avec boot.o pour faire kernel.bin.
• profile.release : panic = "abort", optimisations (opt-level, lto, codegen-units) adaptées à un petit kernel.



--- i386-unknown-none.json ---
Description de la "cible" Rust : processeur 32 bits (i386), pas d’OS ("none"). Le compilateur génère du code pour cette cible au lieu de Linux/Windows. "none" pour l’OS signifie qu’on n’utilise pas la std et qu’on ne link pas avec les libs du système.



--- Makefile ---
Automatise la construction du projet.
• Assemble boot.asm → boot.o (nasm).
• Compile le Rust en lib statique avec la cible i386-unknown-none (cargo + nightly).
• Lie boot.o + la lib Rust avec linker.ld → kernel.bin (ld).
• Copie kernel.bin et grub.cfg dans iso/boot/, puis fabrique l’ISO avec grub-mkrescue.
• make run : lance QEMU avec l’ISO pour tester le kernel.