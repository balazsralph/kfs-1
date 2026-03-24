# Run

Lancer l’ISO dans QEMU :

```bash
make run
```

En ligne de commande :

```bash
qemu-system-i386 -cdrom ./kfs.iso
qemu-system-i386 -kernel ./kernel.bin
```

(`-cdrom` boote via GRUB depuis l’ISO ; `-kernel` charge le binaire directement.)

# Debug

QEMU en pause, prêt pour GDB :

```bash
qemu-system-i386 -cdrom kfs.iso -s -S -no-reboot -no-shutdown
```

Dans un autre terminal, attacher GDB :

```bash
gdb
(gdb) target remote :1234
(gdb) symbol-file kernel.bin
```
