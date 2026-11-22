/* IA: estructura con ??? y pistas de Claude; completado, comentado y probado por nosotros. */

// pantalla.h: se encarga de dibujar en la pantalla; sus funciones se llaman desde el main
#ifndef __PANTALLA_H__
#define __PANTALLA_H__
#include <stdint.h>

void pantalla_inicio(void);                                  // dibuja lo fijo (fondo, barras, titulo); al encender, al salir del menu y al elegir cancion
void pantalla_cancion(uint8_t numero, const char *nombre);   // dibuja el numero y el nombre de la cancion; se llama cuando cambia la cancion
void pantalla_estado(const char *texto);                     // dibuja el estado (Reproduciendo, Pausa, Stop); se llama cuando cambia el estado

void pantalla_menu_abrir(char nombres[][40], uint8_t seleccion);             // dibuja la lista completa con la cancion marcada; al encender y con MENU
void pantalla_menu_mover(char nombres[][40], uint8_t vieja, uint8_t nueva);  // mueve la marca en el menu; se llama con NEXT o PREV

#endif