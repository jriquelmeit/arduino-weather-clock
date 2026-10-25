#include "buzzer_controller.h"

// ----------------------------------------------------
// 1. MELODÍA: Star Wars - La Marcha Imperial
// ----------------------------------------------------
static const MelodyStep MELODY_STAR_WARS_STEPS[] = {
    // Frase 1 (Tema Principal)
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 650 }, { 0, 450 },

    // Frase 2 (Respuesta Aguda)
    { NOTE_E5, 500 }, { 0, 50 },
    { NOTE_E5, 500 }, { 0, 50 },
    { NOTE_E5, 500 }, { 0, 50 },
    { NOTE_F5, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_GS4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 650 }, { 0, 450 },

    // Frase 3 (Salto de Octava y Descenso)
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_A4, 300 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_GS5, 325 }, { 0, 50 },
    { NOTE_G5, 175 }, { 0, 50 },
    { NOTE_FS5, 125 }, { 0, 50 },
    { NOTE_F5, 125 }, { 0, 50 },
    { NOTE_FS5, 250 }, { 0, 250 },
    { NOTE_AS4, 250 }, { 0, 50 },
    { NOTE_DS5, 400 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_CS5, 150 }, { 0, 50 },
    { NOTE_C5, 125 }, { 0, 50 },
    { NOTE_B4, 125 }, { 0, 50 },
    { NOTE_C5, 250 }, { 0, 250 },
    { NOTE_F4, 250 }, { 0, 50 },
    { NOTE_GS4, 450 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 450 }, { 0, 50 },
    { NOTE_A4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_E5, 650 }, { 0, 450 },

    // Frase 4 (Variación Final y Cierre)
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_A4, 300 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_A5, 500 }, { 0, 50 },
    { NOTE_GS5, 325 }, { 0, 50 },
    { NOTE_G5, 175 }, { 0, 50 },
    { NOTE_FS5, 125 }, { 0, 50 },
    { NOTE_F5, 125 }, { 0, 50 },
    { NOTE_FS5, 250 }, { 0, 250 },
    { NOTE_AS4, 250 }, { 0, 50 },
    { NOTE_DS5, 400 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_CS5, 150 }, { 0, 50 },
    { NOTE_C5, 125 }, { 0, 50 },
    { NOTE_B4, 125 }, { 0, 50 },
    { NOTE_C5, 250 }, { 0, 250 },
    { NOTE_F4, 250 }, { 0, 50 },
    { NOTE_GS4, 450 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 50 },
    { NOTE_F4, 350 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_A4, 750 }, { 0, 800 }
};
static const uint16_t STAR_WARS_LENGTH = sizeof(MELODY_STAR_WARS_STEPS) / sizeof(MELODY_STAR_WARS_STEPS[0]);

// ----------------------------------------------------
// 2. MELODÍA: Super Mario Bros (Overworld Theme)
// ----------------------------------------------------
static const MelodyStep MELODY_MARIO_STEPS[] = {
    // Intro
    { NOTE_E5, 130 }, { 0, 50 },
    { NOTE_E5, 130 }, { 0, 150 },
    { NOTE_E5, 130 }, { 0, 150 },
    { NOTE_C5, 130 }, { 0, 50 },
    { NOTE_E5, 130 }, { 0, 150 },
    { NOTE_G5, 250 }, { 0, 250 },
    { NOTE_G4, 250 }, { 0, 350 },

    // Parte A
    { NOTE_C5, 250 }, { 0, 100 },
    { NOTE_G4, 250 }, { 0, 100 },
    { NOTE_E4, 250 }, { 0, 100 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_B4, 200 }, { 0, 60 },
    { NOTE_AS4, 180 }, { 0, 60 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_G4, 180 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_A5, 250 }, { 0, 60 },
    { NOTE_F5, 180 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_C5, 180 }, { 0, 60 },
    { NOTE_D5, 180 }, { 0, 60 },
    { NOTE_B4, 250 }, { 0, 150 },

    // Parte A (Repetición)
    { NOTE_C5, 250 }, { 0, 100 },
    { NOTE_G4, 250 }, { 0, 100 },
    { NOTE_E4, 250 }, { 0, 100 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_B4, 200 }, { 0, 60 },
    { NOTE_AS4, 180 }, { 0, 60 },
    { NOTE_A4, 200 }, { 0, 60 },
    { NOTE_G4, 180 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_A5, 250 }, { 0, 60 },
    { NOTE_F5, 180 }, { 0, 60 },
    { NOTE_G5, 200 }, { 0, 60 },
    { NOTE_E5, 200 }, { 0, 60 },
    { NOTE_C5, 180 }, { 0, 60 },
    { NOTE_D5, 180 }, { 0, 60 },
    { NOTE_B4, 300 }, { 0, 250 },

    // Parte B (Descensos y saltos)
    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_FS5, 180 }, { 0, 50 },
    { NOTE_F5, 180 }, { 0, 50 },
    { NOTE_DS5, 200 }, { 0, 50 },
    { NOTE_E5, 200 }, { 0, 100 },
    { NOTE_GS4, 150 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 180 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_D5, 220 }, { 0, 150 },

    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_FS5, 180 }, { 0, 50 },
    { NOTE_F5, 180 }, { 0, 50 },
    { NOTE_DS5, 200 }, { 0, 50 },
    { NOTE_E5, 200 }, { 0, 100 },
    { NOTE_C6, 250 }, { 0, 50 },
    { NOTE_C6, 200 }, { 0, 50 },
    { NOTE_C6, 350 }, { 0, 250 },

    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_FS5, 180 }, { 0, 50 },
    { NOTE_F5, 180 }, { 0, 50 },
    { NOTE_DS5, 200 }, { 0, 50 },
    { NOTE_E5, 200 }, { 0, 100 },
    { NOTE_GS4, 150 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 180 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_C5, 150 }, { 0, 50 },
    { NOTE_D5, 250 }, { 0, 150 },

    { NOTE_DS5, 300 }, { 0, 60 },
    { NOTE_D5, 300 }, { 0, 60 },
    { NOTE_C5, 450 }, { 0, 600 }
};
static const uint16_t MARIO_LENGTH = sizeof(MELODY_MARIO_STEPS) / sizeof(MELODY_MARIO_STEPS[0]);

// ----------------------------------------------------
// 3. MELODÍA: The Legend of Zelda (Main Theme Overworld)
// ----------------------------------------------------
static const MelodyStep MELODY_ZELDA_STEPS[] = {
    // Fanfarria inicial
    { NOTE_AS4, 380 }, { 0, 40 },
    { NOTE_F4, 200 }, { 0, 40 },
    { NOTE_AS4, 160 }, { 0, 40 },
    { NOTE_AS4, 100 }, { 0, 40 },
    { NOTE_C5, 100 }, { 0, 40 },
    { NOTE_D5, 100 }, { 0, 40 },
    { NOTE_DS5, 100 }, { 0, 40 },
    { NOTE_F5, 600 }, { 0, 160 },

    // Tema principal
    { NOTE_F5, 240 }, { 0, 40 },
    { NOTE_F5, 160 }, { 0, 40 },
    { NOTE_F5, 100 }, { 0, 40 },
    { NOTE_FS5, 140 }, { 0, 40 },
    { NOTE_GS5, 140 }, { 0, 40 },
    { NOTE_AS5, 600 }, { 0, 160 },

    // Respuesta y bajada
    { NOTE_AS5, 220 }, { 0, 40 },
    { NOTE_AS5, 140 }, { 0, 40 },
    { NOTE_GS5, 140 }, { 0, 40 },
    { NOTE_FS5, 140 }, { 0, 40 },
    { NOTE_GS5, 380 }, { 0, 80 },
    { NOTE_FS5, 180 }, { 0, 40 },
    { NOTE_F5, 600 }, { 0, 500 }
};
static const uint16_t ZELDA_LENGTH = sizeof(MELODY_ZELDA_STEPS) / sizeof(MELODY_ZELDA_STEPS[0]);

// ----------------------------------------------------
// 4. MELODÍA: Tetris (Korobeiniki - Type A)
// ----------------------------------------------------
static const MelodyStep MELODY_TETRIS_STEPS[] = {
    { NOTE_E5, 350 }, { 0, 50 },
    { NOTE_B4, 180 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_D5, 350 }, { 0, 50 },
    { NOTE_C5, 180 }, { 0, 50 },
    { NOTE_B4, 200 }, { 0, 50 },
    { NOTE_A4, 350 }, { 0, 50 },
    { NOTE_A4, 180 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_E5, 350 }, { 0, 50 },
    { NOTE_D5, 180 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_B4, 450 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_D5, 350 }, { 0, 50 },
    { NOTE_E5, 350 }, { 0, 50 },
    { NOTE_C5, 350 }, { 0, 50 },
    { NOTE_A4, 350 }, { 0, 50 },
    { NOTE_A4, 450 }, { 0, 250 },

    // Parte B rápida
    { NOTE_D5, 350 }, { 0, 50 },
    { NOTE_F5, 200 }, { 0, 50 },
    { NOTE_A5, 350 }, { 0, 50 },
    { NOTE_G5, 180 }, { 0, 50 },
    { NOTE_F5, 200 }, { 0, 50 },
    { NOTE_E5, 450 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_E5, 350 }, { 0, 50 },
    { NOTE_D5, 180 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_B4, 350 }, { 0, 50 },
    { NOTE_B4, 180 }, { 0, 50 },
    { NOTE_C5, 200 }, { 0, 50 },
    { NOTE_D5, 350 }, { 0, 50 },
    { NOTE_E5, 350 }, { 0, 50 },
    { NOTE_C5, 350 }, { 0, 50 },
    { NOTE_A4, 350 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 500 }
};
static const uint16_t TETRIS_LENGTH = sizeof(MELODY_TETRIS_STEPS) / sizeof(MELODY_TETRIS_STEPS[0]);

// ----------------------------------------------------
// 5. MELODÍA: Pac-Man (Intro Arcade)
// ----------------------------------------------------
static const MelodyStep MELODY_PACMAN_STEPS[] = {
    { NOTE_B4, 120 }, { 0, 20 },
    { NOTE_B5, 120 }, { 0, 20 },
    { NOTE_FS5, 120 }, { 0, 20 },
    { NOTE_DS5, 120 }, { 0, 20 },
    { NOTE_B5, 100 }, { 0, 20 },
    { NOTE_FS5, 150 }, { 0, 20 },
    { NOTE_DS5, 200 }, { 0, 50 },

    { NOTE_C5, 120 }, { 0, 20 },
    { NOTE_C6, 120 }, { 0, 20 },
    { NOTE_G5, 120 }, { 0, 20 },
    { NOTE_E5, 120 }, { 0, 20 },
    { NOTE_C6, 100 }, { 0, 20 },
    { NOTE_G5, 150 }, { 0, 20 },
    { NOTE_E5, 200 }, { 0, 50 },

    { NOTE_B4, 120 }, { 0, 20 },
    { NOTE_B5, 120 }, { 0, 20 },
    { NOTE_FS5, 120 }, { 0, 20 },
    { NOTE_DS5, 120 }, { 0, 20 },
    { NOTE_B5, 100 }, { 0, 20 },
    { NOTE_FS5, 150 }, { 0, 20 },
    { NOTE_DS5, 200 }, { 0, 50 },

    // Escala cromática final
    { NOTE_DS5, 80 }, { 0, 20 },
    { NOTE_E5, 80 }, { 0, 20 },
    { NOTE_F5, 80 }, { 0, 20 },
    { NOTE_FS5, 80 }, { 0, 20 },
    { NOTE_G5, 80 }, { 0, 20 },
    { NOTE_GS5, 80 }, { 0, 20 },
    { NOTE_A5, 80 }, { 0, 20 },
    { NOTE_AS5, 80 }, { 0, 20 },
    { NOTE_B5, 300 }, { 0, 400 }
};
static const uint16_t PACMAN_LENGTH = sizeof(MELODY_PACMAN_STEPS) / sizeof(MELODY_PACMAN_STEPS[0]);

// ----------------------------------------------------
// 6. MELODÍA: Nokia Tune (Gran Vals)
// ----------------------------------------------------
static const MelodyStep MELODY_NOKIA_STEPS[] = {
    { NOTE_E5, 140 }, { 0, 30 },
    { NOTE_D5, 140 }, { 0, 30 },
    { NOTE_FS4, 250 }, { 0, 50 },
    { NOTE_GS4, 250 }, { 0, 50 },
    { NOTE_CS5, 140 }, { 0, 30 },
    { NOTE_B4, 140 }, { 0, 30 },
    { NOTE_D4, 250 }, { 0, 50 },
    { NOTE_E4, 250 }, { 0, 50 },
    { NOTE_B4, 140 }, { 0, 30 },
    { NOTE_A4, 140 }, { 0, 30 },
    { NOTE_CS4, 250 }, { 0, 50 },
    { NOTE_E4, 250 }, { 0, 50 },
    { NOTE_A4, 500 }, { 0, 500 }
};
static const uint16_t NOKIA_LENGTH = sizeof(MELODY_NOKIA_STEPS) / sizeof(MELODY_NOKIA_STEPS[0]);

// ----------------------------------------------------
// 7. MELODÍA: Star Wars - Cantina Band
// ----------------------------------------------------
static const MelodyStep MELODY_CANTINA_STEPS[] = {
    { NOTE_A4, 200 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_A4, 200 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_D5, 150 }, { 0, 50 },
    { NOTE_A4, 200 }, { 0, 100 },

    { NOTE_GS4, 180 }, { 0, 40 },
    { NOTE_A4, 180 }, { 0, 40 },
    { NOTE_A4, 180 }, { 0, 40 },
    { NOTE_GS4, 180 }, { 0, 40 },
    { NOTE_A4, 180 }, { 0, 40 },
    { NOTE_G4, 250 }, { 0, 100 },

    { NOTE_FS4, 180 }, { 0, 40 },
    { NOTE_G4, 180 }, { 0, 40 },
    { NOTE_FS4, 180 }, { 0, 40 },
    { NOTE_F4, 250 }, { 0, 50 },
    { NOTE_D4, 350 }, { 0, 250 },

    { NOTE_A4, 200 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_A4, 200 }, { 0, 50 },
    { NOTE_D5, 200 }, { 0, 50 },
    { NOTE_A4, 150 }, { 0, 50 },
    { NOTE_D5, 150 }, { 0, 50 },
    { NOTE_A4, 200 }, { 0, 100 },

    { NOTE_GS4, 180 }, { 0, 40 },
    { NOTE_A4, 180 }, { 0, 40 },
    { NOTE_C5, 250 }, { 0, 50 },
    { NOTE_B4, 250 }, { 0, 50 },
    { NOTE_A4, 250 }, { 0, 50 },
    { NOTE_G4, 250 }, { 0, 50 },
    { NOTE_E4, 450 }, { 0, 450 }
};
static const uint16_t CANTINA_LENGTH = sizeof(MELODY_CANTINA_STEPS) / sizeof(MELODY_CANTINA_STEPS[0]);

// ----------------------------------------------------
// 8. MELODÍA: Doom (E1M1 / At Doom's Gate)
// ----------------------------------------------------
static const MelodyStep MELODY_DOOM_STEPS[] = {
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E4, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_D4, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_C4, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_AS3, 150 }, { 0, 40 },
    { NOTE_B3, 150 }, { 0, 40 },

    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E4, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_D4, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_C4, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_E3, 140 }, { 0, 40 },
    { NOTE_B3, 200 }, { 0, 50 },
    { NOTE_C4, 220 }, { 0, 300 }
};
static const uint16_t DOOM_LENGTH = sizeof(MELODY_DOOM_STEPS) / sizeof(MELODY_DOOM_STEPS[0]);

// ----------------------------------------------------
// 9. MELODÍA: Alarma Digital / Casio Beep
// ----------------------------------------------------
static const MelodyStep MELODY_CASIO_STEPS[] = {
    // Ráfaga 1: bip-bip suave
    { NOTE_B5, 80 }, { 0, 60 },
    { NOTE_B5, 80 }, { 0, 350 },

    // Ráfaga 2: bip-bip suave
    { NOTE_B5, 80 }, { 0, 60 },
    { NOTE_B5, 80 }, { 0, 350 },

    // Ráfaga 3: cuatro bips rápidos
    { NOTE_B5, 70 }, { 0, 50 },
    { NOTE_B5, 70 }, { 0, 50 },
    { NOTE_B5, 70 }, { 0, 50 },
    { NOTE_B5, 70 }, { 0, 250 },

    // Ráfaga 4: cuatro bips continuos
    { NOTE_B5, 70 }, { 0, 50 },
    { NOTE_B5, 70 }, { 0, 50 },
    { NOTE_B5, 70 }, { 0, 50 },
    { NOTE_B5, 70 }, { 0, 500 }
};
static const uint16_t CASIO_LENGTH = sizeof(MELODY_CASIO_STEPS) / sizeof(MELODY_CASIO_STEPS[0]);

// ----------------------------------------------------
// 10. MELODÍA: Volver al Futuro (Back to the Future - Main Theme)
// ----------------------------------------------------
static const MelodyStep MELODY_BTTF_STEPS[] = {
    // Frase 1: Motivo heroico lidio de apertura
    { NOTE_C4, 400 }, { 0, 50 },
    { NOTE_FS4, 550 }, { 0, 50 },
    { NOTE_G4, 250 }, { 0, 50 },
    { NOTE_C5, 750 }, { 0, 150 },

    // Bajada y resolución
    { NOTE_B4, 200 }, { 0, 40 },
    { NOTE_A4, 200 }, { 0, 40 },
    { NOTE_B4, 300 }, { 0, 40 },
    { NOTE_C5, 300 }, { 0, 40 },
    { NOTE_G4, 600 }, { 0, 250 },

    // Frase 2: Segunda llamada heroica elevándose
    { NOTE_C4, 400 }, { 0, 50 },
    { NOTE_FS4, 550 }, { 0, 50 },
    { NOTE_G4, 250 }, { 0, 50 },
    { NOTE_C5, 750 }, { 0, 150 },

    // Variación ascendente al clímax
    { NOTE_B4, 200 }, { 0, 40 },
    { NOTE_A4, 200 }, { 0, 40 },
    { NOTE_B4, 300 }, { 0, 40 },
    { NOTE_C5, 300 }, { 0, 40 },
    { NOTE_D5, 800 }, { 0, 250 },

    // Frase 3: La marcha triunfal (88 MPH)
    { NOTE_E5, 450 }, { 0, 50 },
    { NOTE_D5, 250 }, { 0, 50 },
    { NOTE_C5, 400 }, { 0, 50 },
    { NOTE_B4, 250 }, { 0, 50 },
    { NOTE_A4, 400 }, { 0, 50 },
    { NOTE_G4, 250 }, { 0, 50 },
    { NOTE_G4, 200 }, { 0, 40 },
    { NOTE_A4, 200 }, { 0, 40 },
    { NOTE_B4, 200 }, { 0, 40 },
    { NOTE_C5, 850 }, { 0, 500 }
};
static const uint16_t BTTF_LENGTH = sizeof(MELODY_BTTF_STEPS) / sizeof(MELODY_BTTF_STEPS[0]);

// ----------------------------------------------------
// 11. MELODÍA: Los Simpson (The Simpsons Theme - Danny Elfman)
// ----------------------------------------------------
static const MelodyStep MELODY_SIMPSONS_STEPS[] = {
    // Frase 1: "The Simp-sons..." (Lidio)
    { NOTE_C4, 450 }, { 0, 80 },
    { NOTE_E4, 450 }, { 0, 80 },
    { NOTE_FS4, 450 }, { 0, 80 },
    { NOTE_A4, 600 }, { 0, 150 },

    // Bajada cómica
    { NOTE_G4, 300 }, { 0, 40 },
    { NOTE_E4, 300 }, { 0, 40 },
    { NOTE_C4, 300 }, { 0, 40 },
    { NOTE_A3, 300 }, { 0, 60 },

    // Tres notas repetidas en Fa sostenido
    { NOTE_FS3, 180 }, { 0, 40 },
    { NOTE_FS3, 180 }, { 0, 40 },
    { NOTE_FS3, 180 }, { 0, 40 },
    { NOTE_G3, 500 }, { 0, 250 },

    // Riff rápido
    { NOTE_FS3, 150 }, { 0, 30 },
    { NOTE_FS3, 150 }, { 0, 30 },
    { NOTE_FS3, 150 }, { 0, 30 },
    { NOTE_G3, 300 }, { 0, 50 },
    { NOTE_AS3, 200 }, { 0, 30 },
    { NOTE_B3, 200 }, { 0, 30 },
    { NOTE_C4, 400 }, { 0, 100 },

    // Repetición brillante final
    { NOTE_C4, 250 }, { 0, 30 },
    { NOTE_E4, 250 }, { 0, 30 },
    { NOTE_FS4, 250 }, { 0, 30 },
    { NOTE_A4, 450 }, { 0, 60 },
    { NOTE_G4, 250 }, { 0, 30 },
    { NOTE_E4, 250 }, { 0, 30 },
    { NOTE_C4, 250 }, { 0, 30 },
    { NOTE_A3, 400 }, { 0, 500 }
};
static const uint16_t SIMPSONS_LENGTH = sizeof(MELODY_SIMPSONS_STEPS) / sizeof(MELODY_SIMPSONS_STEPS[0]);

// ----------------------------------------------------
// 12. MELODÍA: Sonic the Hedgehog (Green Hill Zone)
// ----------------------------------------------------
static const MelodyStep MELODY_SONIC_STEPS[] = {
    // Intro enérgica
    { NOTE_G5, 180 }, { 0, 40 },
    { NOTE_E5, 180 }, { 0, 40 },
    { NOTE_F5, 180 }, { 0, 40 },
    { NOTE_G5, 250 }, { 0, 50 },
    { NOTE_E5, 180 }, { 0, 40 },
    { NOTE_F5, 180 }, { 0, 40 },
    { NOTE_G5, 300 }, { 0, 60 },

    // Ascenso de notas
    { NOTE_C5, 200 }, { 0, 40 },
    { NOTE_B4, 200 }, { 0, 40 },
    { NOTE_C5, 200 }, { 0, 40 },
    { NOTE_D5, 200 }, { 0, 40 },
    { NOTE_E5, 200 }, { 0, 40 },
    { NOTE_F5, 350 }, { 0, 100 },

    // Frase principal Green Hill Zone
    { NOTE_G5, 350 }, { 0, 50 },
    { NOTE_E5, 250 }, { 0, 40 },
    { NOTE_D5, 250 }, { 0, 40 },
    { NOTE_C5, 350 }, { 0, 50 },
    { NOTE_B4, 200 }, { 0, 40 },
    { NOTE_C5, 200 }, { 0, 40 },
    { NOTE_D5, 450 }, { 0, 80 },

    { NOTE_C5, 250 }, { 0, 40 },
    { NOTE_B4, 250 }, { 0, 40 },
    { NOTE_A4, 250 }, { 0, 40 },
    { NOTE_G4, 350 }, { 0, 50 },
    { NOTE_A4, 200 }, { 0, 40 },
    { NOTE_B4, 200 }, { 0, 40 },
    { NOTE_C5, 600 }, { 0, 500 }
};
static const uint16_t SONIC_LENGTH = sizeof(MELODY_SONIC_STEPS) / sizeof(MELODY_SONIC_STEPS[0]);

// ----------------------------------------------------
// 13. MELODÍA: Alien (Tema Principal - Jerry Goldsmith)
// ----------------------------------------------------
static const MelodyStep MELODY_ALIEN_STEPS[] = {
    // Frase 1: La soledad inquietante en el espacio profundo
    { NOTE_C5, 400 }, { 0, 80 },
    { NOTE_DS5, 400 }, { 0, 80 },
    { NOTE_D5, 700 }, { 0, 200 },
    { NOTE_GS4, 450 }, { 0, 80 },
    { NOTE_G4, 900 }, { 0, 300 },

    // Frase 2: El eco lejano de la Nostromo
    { NOTE_C5, 400 }, { 0, 80 },
    { NOTE_DS5, 400 }, { 0, 80 },
    { NOTE_D5, 700 }, { 0, 200 },
    { NOTE_AS4, 450 }, { 0, 80 },
    { NOTE_A4, 900 }, { 0, 250 },

    // Frase 3: La tensión del xenomorfo acechando (crescendo)
    { NOTE_CS4, 150 }, { 0, 50 },
    { NOTE_D4, 150 }, { 0, 50 },
    { NOTE_CS4, 150 }, { 0, 50 },
    { NOTE_D4, 150 }, { 0, 50 },
    { NOTE_DS4, 200 }, { 0, 50 },
    { NOTE_E4, 200 }, { 0, 50 },
    { NOTE_F4, 250 }, { 0, 50 },
    { NOTE_FS4, 300 }, { 0, 80 },
    { NOTE_G4, 600 }, { 0, 100 },
    { NOTE_C5, 950 }, { 0, 600 }
};
static const uint16_t ALIEN_LENGTH = sizeof(MELODY_ALIEN_STEPS) / sizeof(MELODY_ALIEN_STEPS[0]);

BuzzerController::BuzzerController(uint8_t pin, uint8_t melodyType)
    : _pin(pin),
      _melodyType(melodyType),
      _isAlarming(false),
      _isBeeping(false),
      _alarmStartTime(0),
      _alarmMaxDuration(0),
      _stepEndTime(0),
      _beepEndTime(0),
      _melodyIndex(0) {}

void BuzzerController::begin() {
    pinMode(_pin, OUTPUT);
    noTone(_pin);
    digitalWrite(_pin, LOW);
}

void BuzzerController::setMelodyType(uint8_t melodyType) {
    _melodyType = melodyType;
}

const MelodyStep* BuzzerController::getActiveMelody() const {
    switch (_melodyType) {
        case MELODY_MARIO:      return MELODY_MARIO_STEPS;
        case MELODY_ZELDA:      return MELODY_ZELDA_STEPS;
        case MELODY_TETRIS:     return MELODY_TETRIS_STEPS;
        case MELODY_PACMAN:     return MELODY_PACMAN_STEPS;
        case MELODY_NOKIA:      return MELODY_NOKIA_STEPS;
        case MELODY_CANTINA:    return MELODY_CANTINA_STEPS;
        case MELODY_DOOM:       return MELODY_DOOM_STEPS;
        case MELODY_CASIO_BEEP: return MELODY_CASIO_STEPS;
        case MELODY_BTTF:       return MELODY_BTTF_STEPS;
        case MELODY_SIMPSONS:   return MELODY_SIMPSONS_STEPS;
        case MELODY_SONIC:      return MELODY_SONIC_STEPS;
        case MELODY_ALIEN:      return MELODY_ALIEN_STEPS;
        case MELODY_STAR_WARS:
        default:                return MELODY_STAR_WARS_STEPS;
    }
}

uint16_t BuzzerController::getActiveMelodyLength() const {
    switch (_melodyType) {
        case MELODY_MARIO:      return MARIO_LENGTH;
        case MELODY_ZELDA:      return ZELDA_LENGTH;
        case MELODY_TETRIS:     return TETRIS_LENGTH;
        case MELODY_PACMAN:     return PACMAN_LENGTH;
        case MELODY_NOKIA:      return NOKIA_LENGTH;
        case MELODY_CANTINA:    return CANTINA_LENGTH;
        case MELODY_DOOM:       return DOOM_LENGTH;
        case MELODY_CASIO_BEEP: return CASIO_LENGTH;
        case MELODY_BTTF:       return BTTF_LENGTH;
        case MELODY_SIMPSONS:   return SIMPSONS_LENGTH;
        case MELODY_SONIC:      return SONIC_LENGTH;
        case MELODY_ALIEN:      return ALIEN_LENGTH;
        case MELODY_STAR_WARS:
        default:                return STAR_WARS_LENGTH;
    }
}

void BuzzerController::playStartupMelody(int stopPin1, int stopPin2, int stopPin3) {
    const MelodyStep* melody = getActiveMelody();
    uint16_t length = getActiveMelodyLength();

    for (uint16_t i = 0; i < length; i++) {
        uint16_t freq = melody[i].freq;
        uint16_t dur = melody[i].duration;

        if (freq > 0) {
            tone(_pin, freq);
        } else {
            noTone(_pin);
        }

        unsigned long startWait = millis();
        while (millis() - startWait < dur) {
            delay(15);
            yield();

            // Si se presiona algún botón durante la prueba, silenciar de inmediato
            if ((stopPin1 >= 0 && digitalRead(stopPin1) == LOW) ||
                (stopPin2 >= 0 && digitalRead(stopPin2) == LOW) ||
                (stopPin3 >= 0 && digitalRead(stopPin3) == LOW)) {
                stop();
                return;
            }
        }
    }
    noTone(_pin);
    digitalWrite(_pin, LOW);
}

void BuzzerController::startAlarm(unsigned long maxDurationMs) {
    const MelodyStep* melody = getActiveMelody();

    _isAlarming = true;
    _isBeeping = false;
    _alarmStartTime = millis();
    _alarmMaxDuration = maxDurationMs;
    _melodyIndex = 0;

    uint16_t freq = melody[0].freq;
    if (freq > 0) {
        tone(_pin, freq);
    } else {
        noTone(_pin);
    }
    _stepEndTime = millis() + melody[0].duration;
}

void BuzzerController::stop() {
    _isAlarming = false;
    _isBeeping = false;
    _melodyIndex = 0;
    noTone(_pin);
    digitalWrite(_pin, LOW);
}

void BuzzerController::beep(unsigned long durationMs, uint16_t freq) {
    if (_isAlarming) return; // Prioridad para la alarma
    _isBeeping = true;
    _beepEndTime = millis() + durationMs;
    tone(_pin, freq);
}

bool BuzzerController::isAlarming() const {
    return _isAlarming;
}

void BuzzerController::update() {
    unsigned long now = millis();

    // 1. Manejo de Beep corto
    if (_isBeeping) {
        if (now >= _beepEndTime) {
            _isBeeping = false;
            noTone(_pin);
            digitalWrite(_pin, LOW);
        }
        return;
    }

    // 2. Manejo de Alarma con notas reales (no bloqueante)
    if (_isAlarming) {
        if ((now - _alarmStartTime) >= _alarmMaxDuration) {
            stop();
            return;
        }

        if (now >= _stepEndTime) {
            const MelodyStep* melody = getActiveMelody();
            uint16_t length = getActiveMelodyLength();

            _melodyIndex++;
            if (_melodyIndex >= length) {
                _melodyIndex = 0;
            }

            uint16_t freq = melody[_melodyIndex].freq;
            if (freq > 0) {
                tone(_pin, freq);
            } else {
                noTone(_pin);
            }
            _stepEndTime = now + melody[_melodyIndex].duration;
        }
    }
}
