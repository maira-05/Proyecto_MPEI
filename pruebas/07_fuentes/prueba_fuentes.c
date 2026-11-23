/*
 * IA (Claude): prueba generada por Claude; la ejecutamos y revisamos en la placa.
 *
 * prueba_fuentes.c - PRUEBA DE FUENTES EN LA PANTALLA (ST7789 240x320 por SPI2)
 * No usa la SD, el audio ni los botones.
 *
 * Objetivo: ver en la pantalla real como se ven varias fuentes de Adafruit GFX
 * y elegir cuales usar en el reproductor.
 * Muestra 4 paginas que cambian cada 4 segundos:
 *   1. Colores basicos (para revisar que los colores salgan bien)
 *   2. Familia Sans (9, Bold 9, 12, Bold 12, Bold 18)
 *   3. Mono, Serif, TomThumb y DigitalWF (reloj)
 *   4. Ejemplo de la pantalla de reproduccion
 *
 * RESULTADO: se eligieron FreeSansBold (9, 12 y 18 pt) para el texto y
 * DigitalWF 24 pt; las demas fuentes no se usaron en el proyecto final.
 *
 * Notas:
 *  - Usa su propia version de tft_text.c (la primera, con tft_draw_char), no la
 *    de tft_fuentes/. Por eso define aqui tft_set_window y tft_push_color.
 *  - Usa los drivers del proyecto sin modificarlos: system.c (SystemInit lo
 *    llama crt0.S, y delay_ms), gpio.c, spi.c y tft.c.
 */
#include <stdint.h>
#include "tft.h"
#include "system.h"

#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSansBold12pt7b.h"
#include "Fonts/FreeSansBold18pt7b.h"
#include "Fonts/FreeMono9pt7b.h"
#include "Fonts/FreeMonoBold12pt7b.h"
#include "Fonts/FreeSerif9pt7b.h"
#include "Fonts/TomThumb.h"
#include "Fonts/DigitalWF24pt7b.h"
#include "tft_text.h"


/* ====== Funciones de ventana y píxel que necesita tft_text.c ======
 * Se arman con tft_comando() y tft_dato() de tft.c, igual que tft_llenar(). */
void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    tft_comando(0x2A);                       /* CASET: columnas x0..x1 */
    tft_dato(x0 >> 8); tft_dato(x0 & 0xFF);
    tft_dato(x1 >> 8); tft_dato(x1 & 0xFF);

    tft_comando(0x2B);                       /* RASET: filas y0..y1 */
    tft_dato(y0 >> 8); tft_dato(y0 & 0xFF);
    tft_dato(y1 >> 8); tft_dato(y1 & 0xFF);

    tft_comando(0x2C);                       /* RAMWR: lo que siga son píxeles */
}

void tft_push_color(uint16_t color)
{
    tft_dato(color >> 8);                    /* byte alto primero */
    tft_dato(color & 0xFF);
}

/* ====== Ayudas para dibujar ====== */

/* Rectángulo lleno */
static void rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    tft_set_window(x, y, x + w - 1, y + h - 1);
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) tft_push_color(color);
}

/* Texto que se detiene antes de salirse por la derecha */
static uint16_t texto(uint16_t x, uint16_t y, const char *s, const GFXfont *f,
                      uint16_t color, uint16_t fondo)
{
    while (*s) {
        char c = *s;
        if ((uint8_t)c < f->first || (uint8_t)c > f->last) c = '?';
        uint8_t adv = f->glyph[(uint8_t)c - f->first].xAdvance;
        if (x + adv > TFT_ANCHO) break;              /* no cabe: cortar */
        x += tft_draw_char(x, y, *s, f, color, fondo);
        s++;
    }
    return x;
}

/* Texto centrado horizontalmente */
static void centrado(uint16_t y, const char *s, const GFXfont *f,
                     uint16_t color, uint16_t fondo)
{
    uint16_t w = tft_text_width(s, f);
    uint16_t x = (w < TFT_ANCHO) ? (TFT_ANCHO - w) / 2 : 0;
    texto(x, y, s, f, color, fondo);
}

/* Una línea de muestra: dibuja y baja el cursor */
static void muestra(uint16_t *y, const char *s, const GFXfont *f, uint16_t color)
{
    texto(4, *y, s, f, color, TFT_NEGRO);
    *y += f->yAdvance + 4;
}

/* ====== Páginas ====== */

static void pagina_colores(void)
{
    const uint16_t colores[] = {TFT_ROJO, TFT_VERDE, TFT_AZUL, TFT_AMARILLO,
                                TFT_CIAN, TFT_BLANCO};
    const char *nombres[] = {"Rojo", "Verde", "Azul", "Amarillo", "Cian", "Blanco"};

    tft_llenar(TFT_NEGRO);
    centrado(6, "Prueba de colores", &FreeSansBold12pt7b, TFT_BLANCO, TFT_NEGRO);
    for (int i = 0; i < 6; i++) {
        uint16_t y = 50 + i * 44;
        rect(10, y, 60, 36, colores[i]);
        texto(84, y + 8, nombres[i], &FreeSans12pt7b, colores[i], TFT_NEGRO);
    }
}

static void pagina_sans(void)
{
    uint16_t y = 6;
    tft_llenar(TFT_NEGRO);
    muestra(&y, "Sans 9: Hola Maira",   &FreeSans9pt7b,      TFT_BLANCO);
    muestra(&y, "Sans Bold 9: CANC01",  &FreeSansBold9pt7b,  TFT_AMARILLO);
    muestra(&y, "Sans 12: Menu",        &FreeSans12pt7b,     TFT_CIAN);
    muestra(&y, "Bold 12: Pausa",       &FreeSansBold12pt7b, TFT_VERDE);
    muestra(&y, "Bold 18",              &FreeSansBold18pt7b, TFT_NARANJA);
    muestra(&y, "0123456789 !?",        &FreeSans9pt7b,      TFT_GRIS);
    muestra(&y, "ABCDEFGHIJKLMNOPQRS",  &FreeSans9pt7b,      TFT_BLANCO);
    muestra(&y, "abcdefghijklmnopqrstu", &FreeSans9pt7b,     TFT_BLANCO);
}

static void pagina_otras(void)
{
    uint16_t y = 6;
    tft_llenar(TFT_NEGRO);
    muestra(&y, "Mono 9: CANC01.WAV",   &FreeMono9pt7b,      TFT_BLANCO);
    muestra(&y, "MonoBold 12",          &FreeMonoBold12pt7b, TFT_AMARILLO);
    muestra(&y, "Serif 9: Reproductor", &FreeSerif9pt7b,     TFT_CIAN);
    muestra(&y, "TomThumb: letra diminuta 0123456789", &TomThumb, TFT_VERDE);
    y += 10;
    muestra(&y, "03:25",                &DigitalWF24pt7b,    TFT_NARANJA);
}

static void pagina_reproductor(void)
{
    tft_llenar(TFT_NEGRO);

    /* Barra superior con el estado */
    rect(0, 0, TFT_ANCHO, 30, TFT_AZUL_OSCURO);
    texto(8, 5, "> Reproduciendo", &FreeSansBold9pt7b, TFT_BLANCO, TFT_AZUL_OSCURO);

    /* Nombre de la canción y número */
    centrado(80,  "CANC12",  &FreeSansBold18pt7b, TFT_AMARILLO, TFT_NEGRO);
    centrado(140, "12 / 40", &FreeSans12pt7b,     TFT_BLANCO,     TFT_NEGRO);

    /* Tiempo y barra de progreso (decorativos) */
    centrado(185, "01:24", &DigitalWF24pt7b, TFT_NARANJA, TFT_NEGRO);
    rect(20, 250, 200, 8, TFT_GRIS);
    rect(20, 250, 80, 8, TFT_VERDE);

    /* Barra inferior con los botones */
    rect(0, TFT_ALTO - 30, TFT_ANCHO, 30, TFT_AZUL_OSCURO);
    texto(6, TFT_ALTO - 26, "Prev  Pausa  Stop  Next", &FreeSans9pt7b,
          TFT_GRIS, TFT_AZUL_OSCURO);
}

int main(void)
{
    /* SystemInit() (84 MHz) ya lo llamó crt0.S antes de main */
    tft_init();

    while (1) {
        pagina_colores();
        delay_ms(4000);
        pagina_sans();
        delay_ms(4000);
        pagina_otras();
        delay_ms(4000);
        pagina_reproductor();
        delay_ms(4000);
    }
}
