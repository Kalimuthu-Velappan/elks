/* Keymap:SE:Suede:Swedish	*/

#ifndef __KEYMAP_SE__
#define __KEYMAP_SE__

#if defined(CONFIG_KEYMAP_SE)

/**************************************************************
 * Swedish keyboard adapted from the German layout by Per     *
 * Olofsson (MagerValp@cling.gu.se). This works fine on my    *
 * laptop (Toshiba T1200). YMMV.                              *
 **************************************************************/

static unsigned char xtkb_scan[] = {
    0, 033, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '+', '\'', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '�', '~', 015, 0202, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', '�',
    '�', '\'', 0200, '<', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '-', 0201, '*',
    0203, ' ', 0204, 0241, 0242, 0243, 0244, 0245,
    0246, 0247, 0250, 0251, 0252, 0205, 0210, 0267,
    0270, 0271, 0211, 0264, 0265, 0266, 0214, 0261,
    0262, 0263, 'O', 0177, 0, 0, '<'
};

static unsigned char xtkb_scan_shifted[] = {
    0, 033, '!', '\"', '#', '$', '%', '&',
    '/', '(', ')', '=', '?', 0140, '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '�', '^', 015, 0202, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', '�',
    '�', '*', 0200, '>', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ';', ':', '_', 0201, '*',
    0203, ' ', 0204, 0221, 0222, 0223, 0224, 0225,
    0226, 0227, 0230, 0231, 0232, 0204, 0213, '7',
    '8', '9', 0211, '4', '5', '6', 0214, '1',
    '2', '3', '0', 0177, 0, 0, '>'
};

static unsigned char xtkb_scan_ctrl_alt[] = {
    0, 033, '1', '@', '�', '4', '5', '6',
    '{', '[', ']', '}', '\\', '\'', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '�', '~', 015, 0202, 0xa0, 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', '�',
    '�', '\'', 0200, '|', 'z', 'x', 'c', 'v',
    'b', 'n', 0xB5, ',', '.', '-', 0201, '*',
    0203, ' ', 0204, 0241, 0242, 0243, 0244, 0245,
    0246, 0247, 0250, 0251, 0252, 0205, 0210, 0267,
    0270, 0271, 0211, 0264, 0265, 0266, 0214, 0261,
    0262, 0263, 'O', 0177, 0, 0, '|'
};

static unsigned char xtkb_scan_caps[84] = {
    0, 033, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '�', '~', 015, 0202, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', '�',
    '�', 0x80, 0200, '<', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '-', 0201, '*',
    0203, ' ', 0204, 0221, 0222, 0223, 0224, 0225,
    0226, 0227, 0230, 0231, 0232, 0204, 0213, '7',
    '8', '9', 0211, '4', '5', '6', 0214, '1',
    '2', '3', '0', 0177
};

#endif

#endif
