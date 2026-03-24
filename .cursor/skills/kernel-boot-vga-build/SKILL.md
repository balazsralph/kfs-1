---
name: kernel-boot-vga-build
description: Write multiboot-compatible ASM bootloader (header, stack, call kmain), freestanding kernel helpers (strlen, strcmp, memset, memcpy), VGA text output at 0xB8000 (print_string, print_char, clear_screen), Makefile for ASM+kernel+link+ISO, and run/debug with QEMU/KVM/GDB. Use when building an i386 freestanding kernel, boot code, VGA driver, or OS toolchain.
---

# Kernel boot, VGA, build & debug

## 1. ASM bootloader (Multiboot)

Produire : **multiboot_header**, **stack_setup**, **call_kmain**.

- **Multiboot header** : section alignée 4, magic `0x1BADB002`, flags (ex. MBALIGN | MEMINFO), checksum `-(MAGIC + FLAGS)`.
- **Stack** : section `.bss` align 16, `resb` pour la pile (ex. 16 Ko), symboles `stack_bottom` / `stack_top`.
- **Entry** : `global start` (ou _start selon linker), `extern kmain`. Mettre `esp` sur `stack_top`, optionnellement pousser multiboot args (eax, ebx), `call kmain`, puis boucle `cli; hlt; jmp` en cas de retour.

Architecture cible i386, GRUB multiboot. Point d’entrée noyau : `kmain`.

---

## 2. Noyau freestanding (kernel_core)

Implémenter en C ou Rust **sans** stdlib : `strlen`, `strcmp`, `memset`, `memcpy`.

- **strlen** : parcourir jusqu’à `\0`, retourner le compteur.
- **strcmp** : comparer octet par octet, retourner &lt;0 / 0 / &gt;0.
- **memset** : remplir une zone avec un octet (ou mot si alignement garanti).
- **memcpy** : copie octet par octet (ou par mot) de src vers dest.

Flags compilateur typiques : `-ffreestanding`, `-fno-builtin`, `-nostdlib`, `-nodefaultlibs`. Aucune librairie hôte.

---

## 3. Sortie VGA (vga_output)

Adresse framebuffer texte VGA : **0xB8000**.

- **Attribut** : octet haut = couleur (ex. 0x07 gris sur noir). Octet bas = code caractère.
- **print_char** : écrire un caractère + attribut à la position courante, avancer curseur, gérer retour à la ligne / scroll si nécessaire.
- **print_string** : boucle sur une chaîne null-terminated, appeler print_char pour chaque caractère.
- **clear_screen** : remplir les 80×25×2 octets avec espace (0x20) + attribut (ex. 0x07).

Pas de dépendance stdio ; écriture directe en mémoire.

---

## 4. Build (Makefile)

Cibles à couvrir :

- **compile_asm** : assembler le boot (ex. `boot/boot.asm` → `boot.o`) avec nasm/gas, flags pour objet elf32.
- **compile_kernel** : compiler C/Rust du kernel avec flags freestanding, sans stdlib.
- **link_kernel** : lier `boot.o` + objets kernel avec script linker (ex. `linker/linker.ld`), sortie elf (ex. `kernel.bin`). L’entrée doit être le symbole du boot (ex. `start`).
- **create_iso** : générer une iso bootable (ex. GRUB) contenant le kernel et `grub.cfg` (module multiboot + chainload).

Ordre typique : asm → kernel → link → iso.

---

## 5. Debug / VM (debugging_vm)

Exécuter le noyau en environnement virtuel :

- **QEMU** : lancer l’iso ou le kernel (ex. `qemu-system-i386 -kernel kernel.bin` ou `-cdrom image.iso`). Option `-no-reboot -no-shutdown` pour ne pas redémarrer.
- **KVM** : ajouter `-enable-kvm` si disponible sur la machine hôte (accélération).
- **GDB** : QEMU avec `-s -S` (écoute GDB, pause au démarrage) ; dans GDB : `target remote :1234`, puis chargement des symboles du kernel pour le debug.

Utiliser ces outils quand on teste le boot, le VGA ou le noyau sans matériel réel.
