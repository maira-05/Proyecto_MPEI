/* IA: estructura con ??? y pistas de Claude; completado, comentado y probado por nosotros. */

#include "tft_text.h"
#include "tft.h"

// ===== Ascenso de la fuente =====
// Devuelve cuántos píxeles sube la letra más alta por encima de la línea base.
static int8_t ascenso(const GFXfont *fuente)
{
    int8_t min = 0;
    for (uint16_t i = 0; i <= fuente->last - fuente->first; i++)   // Recorre todos los glifos de la fuente
    {
        if (fuente->glyph[i].yOffset < min) min = fuente->glyph[i].yOffset;   // yOffset negativo = sube; guarda el más negativo
    }
    return (int8_t)(-min);   // Cambia el signo para devolver una altura positiva
}

// ===== Búsqueda del glifo =====
// Devuelve el puntero al glifo (ficha con tamaño, offsets y avance) del carácter c.
static const GFXglyph *glifo(char c, const GFXfont *fuente)
{
    if ((uint8_t)c < fuente->first || (uint8_t)c > fuente->last) c = '?';   // Fuera del rango (ej. 'á'): usa '?'
    return &fuente->glyph[(uint8_t)c - fuente->first];                      // Resta 'first' porque la tabla empieza en ese carácter
}

// ===== Dibujar una letra =====
// Dibuja c con su esquina superior izquierda en (x, y) y devuelve su avance en x (0 si no cabe).
uint16_t tft_letra(uint16_t x, uint16_t y, char c, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo)
{
    const GFXglyph *g = glifo(c, fuente);
    uint8_t ancho_celda = g->xAdvance;          // Ancho de la celda (lo que ocupa la letra con su espacio)
    uint8_t alto_celda  = fuente->yAdvance;     // Alto de la celda, igual para todas las letras
    int8_t  base        = ascenso(fuente);      // Fila de la celda donde está la línea base

    if (x + ancho_celda > TFT_ANCHO || y + alto_celda > TFT_ALTO) return 0;   // Se sale de la pantalla: no dibuja y avisa con 0

    tft_ventana(x, y, x + ancho_celda - 1, y + alto_celda - 1);   // Define el rectángulo de pantalla donde se enviarán los píxeles

    for (int cy = 0; cy < alto_celda; cy++)          // Recorre la celda fila por fila (cy, cx = posición en la celda)
    {
        int gy = cy - (base + g->yOffset);           // Fila dentro del molde (el molde empieza en la fila base + yOffset)
        for (int cx = 0; cx < ancho_celda; cx++)
        {
            int gx = cx - g->xOffset;                // Columna dentro del molde
            uint16_t px = fondo;                     // Por defecto el píxel es de fondo

            if (gx >= 0 && gx < g->width && gy >= 0 && gy < g->height)   // Solo si está dentro del molde de la letra
            {
                // n = número de bit dentro del arreglo de bitmaps (inicio del glifo + fila*ancho + columna)
                uint32_t n = (uint32_t)g->bitmapOffset * 8u
                           + (uint32_t)gy * g->width + (uint32_t)gx;
                // Lee ese bit: n>>3 es el byte, 0x80>>(n&7) es el bit dentro del byte; si es 1, usa color
                if (fuente->bitmap[n >> 3] & (0x80u >> (n & 7u))) px = color;
            }
            tft_pixel(px);                           // Envía el píxel a la ventana definida, en orden
        }
    }
    return ancho_celda;
}

// ===== Dibujar un texto =====
// Dibuja la cadena s desde (x, y) y devuelve la x donde termina.
uint16_t tft_texto(uint16_t x, uint16_t y, const char *s, const GFXfont *fuente,
                   uint16_t color, uint16_t fondo)
{
    while (*s)                                       // Se detiene en el '\0' final de la cadena
    {
        uint16_t avance = tft_letra(x, y, *s, fuente, color, fondo);
        if (avance == 0) break;                      // La letra no cabió en pantalla: corta el texto
        x += avance;                                 // Mueve x a donde empieza la siguiente letra
        s++;                                         // Pasa al siguiente carácter
    }
    return x;
}

// ===== Ancho de un texto =====
// Calcula el ancho en píxeles de s sin dibujarlo.
uint16_t tft_ancho_texto(const char *s, const GFXfont *fuente)
{
    uint16_t ancho = 0;
    while (*s)
    {
        ancho += glifo(*s, fuente)->xAdvance;        // Suma el avance de cada letra
        s++;
    }
    return ancho;
}

// ===== Texto centrado =====
// Dibuja s centrado horizontalmente; la x se calcula con el ancho del texto.
void tft_texto_centrado(uint16_t y, const char *s, const GFXfont *fuente,
                        uint16_t color, uint16_t fondo)
{
    uint16_t ancho = tft_ancho_texto(s, fuente);
    uint16_t x = (ancho < TFT_ANCHO) ? (TFT_ANCHO - ancho) / 2 : 0;   // Si es más ancho que la pantalla, empieza en 0
    tft_texto(x, y, s, fuente, color, fondo);
}