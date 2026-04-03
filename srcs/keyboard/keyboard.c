#include <stddef.h>
#include <stdint.h>
#include "keyboard.h"

// Port de données PS/2
#define PS2_DATA    0x60U

// Port d'état du controleur PS/2 pour dire si un octet est disponible à lire dans 0x60, si bit 0 alors buffer de sortie plein donc octet dispo,
// si bit 1 alors buffer d'entrée plein controleur occupé ne faut pas écrire
#define PS2_STATUS  0x64U

#define SC_EXT_DELETE       0x53U
#define SC_EXT_DELETE_SET2  0x71U
#define SC_EXT_LEFT         0x4BU
#define SC_EXT_RIGHT        0x4DU
#define SC_EXT_LEFT_SET2    0x6BU
#define SC_EXT_RIGHT_SET2   0x74U

static const char scancode_map[128] = {
    [0x01] = 0,
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x39] = ' ',
};

static const char shift_num[10] = {
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'
};


/* Lecture d'un octet sur un port d'E/S x86 (inb). */
static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}


/* Applique Shift et Verr. maj. au caractère issu du scancode (layout US QWERTY). */
static char apply_shift_caps(uint8_t sc, char c, uint8_t shift, uint8_t caps)
{
    if (c >= 'a' && c <= 'z')
    {
        if (caps != shift)
            c = (char)(c - 32);
        return c;
    }

    if (!shift)
        return c;

    if (sc >= 0x02U && sc <= 0x0BU)
        return shift_num[(sc - 0x02U)];

    switch (sc)
    {
        case 0x0CU:
            return '_';
        case 0x0DU:
            return '+';
        case 0x1AU:
            return '{';
        case 0x1BU:
            return '}';
        case 0x27U:
            return ':';
        case 0x28U:
            return '"';
        case 0x29U:
            return '~';
        case 0x2BU:
            return '|';
        case 0x33U:
            return '<';
        case 0x34U:
            return '>';
        case 0x35U:
            return '?';
        default:
            return c;
    }
}

/* Vide le buffer du contrôleur PS/2 pour éviter des événements fantômes au démarrage. */
void keyboard_init(void)
{
    // Tant que le bit 0 est à 1, donc octet dispo en en sortie et occupé en entrée"
    while ((inb(PS2_STATUS) & 1U) != 0U)
        (void)inb(PS2_DATA);
}

/* Attend qu'un octet soit disponible puis le lit depuis le port données (0x60). */
static uint8_t keyboard_read_data(void)
{
    while ((inb(PS2_STATUS) & 1U) == 0U)
        __asm__ volatile("pause" ::: "memory");
    return inb(PS2_DATA);
}

char keyboard_getchar(void)
{
    static uint8_t shift_l;
    static uint8_t shift_r;
    uint8_t shift;

    static uint8_t caps;
    uint8_t sc;
    char    c;

    /* Si aucun octet n'est disponible dans le buffer PS/2, on sort.
     * => Fonction non bloquante : elle renvoie 0 si rien à lire.
     */
    if ((inb(PS2_STATUS) & 1U) == 0U)
        return 0;

    /* Lit le scancode principal du clavier. */
    sc = inb(PS2_DATA);

    /* 0xE0 = préfixe "scancode étendu" (flèches, Delete, etc.). */
    if (sc == 0xE0U)
    {
        uint8_t sc2;

        /* Lit le 2e octet de la séquence étendue. */
        sc2 = keyboard_read_data();

        /* Si bit 7 = 1, c'est un "break code" (relâchement), on ignore. */
        if ((sc2 & 0x80U) != 0U)
            return 0;

        /* Touches étendues gérées explicitement. */
        if (sc2 == SC_EXT_DELETE || sc2 == SC_EXT_DELETE_SET2)
            return (char)127;
        if (sc2 == SC_EXT_LEFT || sc2 == SC_EXT_LEFT_SET2)
            return (char)(unsigned char)KBD_ARROW_LEFT;
        if (sc2 == SC_EXT_RIGHT || sc2 == SC_EXT_RIGHT_SET2)
            return (char)(unsigned char)KBD_ARROW_RIGHT;

        /* Autres scancodes étendus non gérés. */
        return 0;
    }

    /* 0xE1 = séquence spéciale (ex: Pause/Break), ici ignorée. */
    if (sc == 0xE1U)
    {
        unsigned int i;

        /* Consomme le reste de la séquence pour nettoyer le flux. */
        for (i = 0U; i < 5U; i++)
            (void)keyboard_read_data();
        return 0;
    }

    /* Si bit 7 = 1, c'est un break code (relâchement d'une touche). */
    if ((sc & 0x80U) != 0U)
    {
        /* Enlève le bit 7 pour retrouver le code "make" de la touche. */
        sc = (uint8_t)(sc & 0x7FU);

        /* Mise à jour de l'état Shift au relâchement. */
        if (sc == 0x2AU)
            shift_l = 0U;
        else if (sc == 0x36U)
            shift_r = 0U;
        return 0;
    }

    /* Make codes (appui) pour Shift gauche / droite et Caps Lock. */
    if (sc == 0x2AU) {
        shift_l = 1U;
        return 0;
    }
    if (sc == 0x36U) {
        shift_r = 1U;
        return 0;
    }
    if (sc == 0x3AU) {
        /* Caps Lock bascule son état (0 -> 1 ou 1 -> 0). */
        caps = (uint8_t)(caps ^ 1U);
        return 0;
    }

    /* Shift actif si au moins une des deux touches Shift est enfoncée. */
    shift = (shift_l | shift_r) ? 1U : 0U;

    /* Sécurité index tableau (même si normalement déjà couvert). */
    if (sc >= 128U)
        return 0;

    c = scancode_map[sc];
    if (c == 0)
        return 0;

    /* Applique Shift/Caps puis renvoie le caractère final. */
    return apply_shift_caps(sc, c, shift, caps);
}
