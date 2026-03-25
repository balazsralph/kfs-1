# Build

Compilation boot ASM + noyau C (freestanding), link avec `linker.ld`, génération ISO :

```bash
make
```

Produit `kfs.iso` (quelques Mo, **sous la limite 10 Mo** du sujet). Le Makefile appelle `grub2-mkrescue` ou `grub-mkrescue` avec `--compress=xz`, `--core-compress=xz` et sans thème / locales / polices GRUB par défaut (`--themes= --locales= --fonts=`), ce qui évite l’image « éléphant » (~25 Mo) obtenue avec les réglages par défaut.

Générer l’ISO à la main avec les mêmes options (répertoire `iso/` déjà rempli comme après `make`) :

```bash
grub2-mkrescue -o kfs.iso --compress=xz --core-compress=xz --themes= --locales= --fonts= iso
```

Nettoyer les artefacts de build :

```bash
make clean
```

Compiler uniquement le noyau lié (objets dans `build/`, puis link) :

```bash
make kernel.bin
```

Les fichiers `.o` sont produits sous **`build/`** (`build/boot.o`, `build/kernel.o`, `build/vga.o`), pas à côté des sources.

Assemble le stub multiboot (`boot/boot.asm` → `build/boot.o`) :

```bash
mkdir -p build
nasm -f elf32 boot/boot.asm -o build/boot.o
```

Compile un fichier C du noyau (exemple `srcs/kernel/kernel.c`, sortie dans `build/`) :

```bash
mkdir -p build
gcc -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs -fno-pie -O2 -Wall -Wextra -Isrcs/include -c srcs/kernel/kernel.c -o build/kernel.o
```
