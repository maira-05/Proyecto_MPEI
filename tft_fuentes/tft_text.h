#ifndef __TFT_TEXT_H__          // Evita redefiniciones si dos archivos incluyen este .h 
#define __TFT_TEXT_H__
#include <stdint.h>
#include "gfxfont.h"            // Define el tipo GFXfont (bitmaps y datos de cada glifo)

/* (x, y) es la esquina superior izquierda de la celda de la letra.
Devuelve el ancho de la celda (cuanto avanzar), o 0 si la letra no cabe en la pantalla.*/
uint16_t tft_letra(uint16_t x, uint16_t y, char c, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo);
/* Dibuja la cadena s desde (x, y). Devuelve la x donde termina el texto
(se detiene antes si una letra no cabe en la pantalla).*/
uint16_t tft_texto(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo);
// Devuelve el ancho en píxeles de la cadena s con esa fuente (no dibuja).
uint16_t tft_ancho_texto(const char *s, const GFXfont *fuente);
/* Dibuja la cadena s centrada horizontalmente en la fila y
(si es mas ancha que la pantalla, empieza en x = 0).*/
void     tft_texto_centrado(uint16_t y, const char *s, const GFXfont *fuente,
                            uint16_t color, uint16_t fondo);

#endif