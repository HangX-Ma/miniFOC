#include "vkey.h"

BOOL g_vkeys[VKEY_ID_SIZE];

VKeyID vkey_scan(void) {
    for (int i = VKEY_ID_START; i < VKEY_ID_SIZE; i++) {
        if (g_vkeys[i] == TRUE) {
            g_vkeys[i] = FALSE; // reset
            return (VKeyID)i;
        }
    }
    return VKEY_ID_NONE;
}

void vkey_init(void) {
    for(int i = VKEY_ID_START; i < VKEY_ID_SIZE; i++) {
        g_vkeys[i] = FALSE;
    }
}
