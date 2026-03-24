# Build

Compilation boot ASM + noyau Rust (lib), link, génération ISO :

```bash
make
```

Produit `kfs.iso`. Le Makefile utilise `grub2-mkrescue` (Fedora) ou `grub-mkrescue` (Debian) selon ce qui est installé.

Nettoyer les artefacts de build :

```bash
make clean
```
