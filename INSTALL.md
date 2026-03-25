# Installation

## Dependencies

Outils de build et émulateur (Debian/Ubuntu) :

```bash
sudo apt install nasm qemu-system-x86 grub-pc-bin xorriso gcc-multilib
```

Fedora :

```bash
sudo dnf install nasm qemu-system-x86 grub2-tools xorriso glibc-devel.i686 libgcc.i686
```

(`gcc-multilib` / `glibc-devel.i686` permettent à `gcc -m32` de produire des objets i386 pour le noyau.)
