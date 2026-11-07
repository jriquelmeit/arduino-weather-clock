#pragma once

#include <Arduino.h>

// Íconos monocromáticos en mapa de bits (1-bit por píxel, PROGMEM)

// Logo de Spotify (12x12 píxeles)
const unsigned char epd_bitmap_spotify_logo[] PROGMEM = {
    0x0f, 0x00, 
    0x3f, 0xc0, 
    0x7f, 0xe0, 
    0x71, 0xe0, 
    0xef, 0x70, 
    0xdf, 0x70, 
    0xbf, 0xd0, 
    0xbf, 0xd0, 
    0x7f, 0xe0, 
    0x7f, 0xe0, 
    0x3f, 0xc0, 
    0x0f, 0x00
};

// Ícono Anterior / Previous (8x8 píxeles)
const unsigned char epd_bitmap_prev[] PROGMEM = {
    0x98, 
    0x9c, 
    0x9e, 
    0x9f, 
    0x9f, 
    0x9e, 
    0x9c, 
    0x98
};

// Ícono Siguiente / Next (8x8 píxeles)
const unsigned char epd_bitmap_next[] PROGMEM = {
    0x19, 
    0x39, 
    0x79, 
    0xf9, 
    0xf9, 
    0x79, 
    0x39, 
    0x19
};

// Ícono Play (triángulo para dentro del círculo, 6x8)
const unsigned char epd_bitmap_play[] PROGMEM = {
    0x30, 
    0x70, 
    0xf0, 
    0xf0, 
    0xf0, 
    0xf0, 
    0x70, 
    0x30
};

// Ícono Pause (dos barras, 6x8)
const unsigned char epd_bitmap_pause[] PROGMEM = {
    0xdb, 
    0xdb, 
    0xdb, 
    0xdb, 
    0xdb, 
    0xdb, 
    0xdb, 
    0xdb
};
