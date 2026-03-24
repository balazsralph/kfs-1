# Installation

## Dependencies

Outils de build et émulateur (Debian/Ubuntu) :

```bash
sudo apt install nasm qemu-system-x86 grub-pc-bin xorriso
```

Fedora :

```bash
sudo dnf install nasm qemu-system-x86 grub2-tools xorriso
```

Rust (rustup obligatoire pour `cargo +nightly`) :

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

Activer Cargo dans la session courante (ou redémarrer le shell) :

```bash
. "$HOME/.cargo/env"
```

Toolchain nightly et sources pour build-std :

```bash
rustup toolchain install nightly
rustup component add rust-src --toolchain nightly-x86_64-unknown-linux-gnu
```

(Nécessaire pour `-Z build-std=core`.)