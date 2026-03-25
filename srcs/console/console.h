#ifndef CONSOLE_H
#define CONSOLE_H

/*
 * Bascule entre consoles virtuelles (0 et 1) : sauvegarde / restaure
 * le tampon texte VGA 80×25 et la position du curseur logique.
 */
void console_switch(int id);

#endif
