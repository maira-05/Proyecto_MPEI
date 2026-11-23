# Uso de inteligencia artificial

Este documento explica qué herramientas de IA se emplearon en el proyecto, con qué
fin, qué partes del código tuvieron su ayuda y cómo se verificó el resultado.

## 1. Herramientas

| Herramienta                        | Uso                                                                                  |
|----------------------------------- |------------------------------------------------------------------------------------- |
| **Claude** en dos conversaciones   | Apoyo en diseño, código, depuración, comentarios, diagramas y documentación          |
| **OpenCode**                       | Descarga de las canciones en WAV y diagnóstico del problema de lectura de la microSD |

Conversaciones con Claude:
1. **"Proyecto MPEI reproductor de música"**: base del reproductor (arranque, linker,
   drivers, microSD, FatFs, audio, doble buffer, botones y estados), depuración del
   hardware, revisión de comentarios de los drivers, diagramas y parte del informe.
2. **Pantalla y documentación**: fuentes, interfaz, menú, comentarios, README y repositorio.

## 2. Qué se hizo con IA

### 2.1 Base del reproductor (conversación 1)

Nosotros definimos la lógica del reproductor: el diagrama de flujo inicial, los cinco
botones, el menú y el botón de encendido. La ayuda de la IA se dio de cuatro maneras:

| Modalidad                                                                   | Archivos                                                  |
|---------------------------------------------------------------------------- |---------------------------------------------------------- |
| **Generado por la IA** (revisado, compilado y probado por nosotros)         | `linker_script.ld`, `crt0.S`                              |
| **Estructura con `???` y pistas** (nosotros completamos y probamos)         | `audio.c`, partes de `main.c`                             |
| **Escrito por nosotros con asesoría** (la IA señalaba errores y su causa)   | `system.c`, `gpio.c`, `uart.c`, `spi.c`, `sd.c`, `tft.c`  |
| **Hecho por nosotros, revisado con la IA** (la IA confirmó que estaba bien) | `diskio.c`, `Makefile`                                    |

Detalles:
- `crt0.S` se adaptó del startup de ST, con `SystemInit` y `_init` débiles.
- `diskio.c` lo adaptamos a partir de la plantilla y los comentarios de FatFs.
- El `Makefile` lo hicimos a partir de los ejemplos del profesor.
- En `main.c`, las partes con `???` fueron el doble buffer, la interrupción de TIM3,
  `buscar_data`, `iniciar_cancion`, los botones (flanco y antirrebote), los estados y el
  botón de encendido.

La IA entregó completos, dentro de `main.c`, el volumen por software
(`VOLUMEN_SHIFT`) y el filtro pasa-altos digital (`FILTRO_K`). Los valores finales los
elegimos nosotros al escuchar el resultado.

También nos orientó en la depuración del hardware, indicando qué medir y probar:
el conflicto de PB3 (SWO) con el SPI, el apagón al alimentar el amplificador desde la
Nucleo, la salida en puente del GF1002, la saturación del audio y los problemas de
tierra y contacto. Las mediciones, el recableado y las pruebas fueron nuestros.

La microSD fallaba al leer (se notó al integrar el amplificador). Con ayuda de la IA
(OpenCode) se encontraron dos causas:
- Los pines del SPI1 estaban en velocidad baja (`OSPEEDR`): los flancos eran demasiado
  lentos para los 5,25 MHz del SPI y se leía mal el último bit de cada byte. Se cambiaron
  a velocidad muy alta.
- El argumento de ACMD41 se completó con la ventana de voltaje (2,7–3,6 V, `0x00FF8000`),
  sin la cual esta tarjeta no terminaba de inicializarse.

Nosotros probamos el arreglo en el montaje completo.

### 2.2 Programas de prueba (`pruebas/`)

La IA entregó completos los programas de prueba de hardware (`01_pines` a
`07_fuentes`), incluido el arreglo de audio de `06_audio_flash/src/cancion.c`. Con ellos probamos cada
componente por separado (pines, pantalla, parlante, microSD, UART, audio y fuentes) antes de
integrarlo al reproductor.

`04_sd` y `05_sd_uart` no lograron iniciar la SD. El módulo y la tarjeta se verificaron
con un Arduino usando el ejemplo de la librería SD de Arduino (no generado por IA).

### 2.3 Pantalla, texto y menú (conversación 2)

La IA no entregó estos archivos completos. El proceso fue:

1.4 Explicaba el concepto (ventana de la ST7789 con CASET, RASET y RAMWR; colores RGB565;
   formato GFXfont; cómo ubicar el bit de cada píxel de una letra).
2. Entregaba el código con `???` y pistas en las partes clave.
3. Nosotros completábamos, compilábamos y probábamos en la placa.
4. La IA revisaba lo completado y explicaba los errores.

| Archivo                             | Contenido                                                                           |
|------------------------------------ |------------------------------------------------------------------------------------ |
| `src/tft.c`, `inc/tft.h`            | `tft_ventana`, `tft_pixel`, `tft_rectangulo`, `tft_llenar` y colores                |
| `tft_fuentes/tft_text.c/.h`         | Dibujo de letras y texto con GFXfont, texto centrado                                |
| `src/pantalla.c`, `inc/pantalla.h`  | Pantalla de reproducción, división del nombre en líneas y menú con desplazamiento   |
| `main.c` (menú y nombres)           | `cargar_nombres`, `abrir_menu`, `salir_menu`, `boton_en_menu` y arranque en el menú |

Otros aportes de esta conversación:
- **Prueba de fuentes** (`pruebas/07_fuentes`): programa completo entregado por la IA
  para ver las fuentes en la pantalla y elegir las del reproductor.
- **SPI2**: la IA sugirió subir la velocidad a 10,5 MHz (prescaler /4) para que el
  dibujo no interfiriera con la reproducción.
- **`NOMBRES.TXT`**: la IA armó la lista de los 40 títulos y nosotros confirmamos el orden.

### 2.4 Comentarios del código

- **Drivers** (`system.c`, `gpio.c`, `uart.c`, `spi.c`, `sd.c`, `diskio.c`, `audio.c`,
  `tft.c`): los escribimos nosotros. En la conversación 1 la IA los revisó uno por uno,
  marcó los incorrectos o incompletos y sugirió textos (casi completos en `uart.c`,
  frases sueltas en el resto), que adaptamos.
- **`pantalla.c` y `pantalla.h`**: los escribimos nosotros. La IA marcaba con `???` los
  que faltaban, daba pistas y luego revisaba lo escrito.
- **`main.c`**: los del menú (`abrir_menu`, `salir_menu`, `boton_en_menu`) son nuestros.
  En la revisión final la IA señaló los poco claros o vacíos y propuso el texto de los
  faltantes (interrupción de TIM3, `buscar_data`, `detener_audio`, `volatile`, botón azul
  y estados). Revisamos cada uno antes de aceptarlo. El código no cambió, solo los comentarios.

### 2.5 Documentación

- **README.md**: el contenido es nuestro (materiales, conexiones, valores del filtro,
  alimentación, pines, uso y problemas encontrados). La IA propuso la estructura de
  secciones y corrigió errores. Para las partes técnicas (compilación y grabación, uso de
  memoria, linker script y tabla de pruebas) dio un borrador con nuestros datos, que
  reescribimos con nuestras palabras.
- **Diagramas del informe** (conversación 1): la IA explicó qué diagramas hacer y cómo
  organizarlos, y dio el pseudocódigo del `main` y un diagrama de flujo de referencia.
  Los diagramas finales (arquitectura por capas, esquemático y flujo del `main`) los
  hicimos nosotros y la IA los revisó.
- **Informe**: en la conversación 1 la IA explicó el diagrama de flujo por etapas. En la
  conversación 2 revisó y corrigió todo el informe, reescribió la sección "Lógica del
  reproductor" y redactó un primer borrador del resumen, la introducción, los resultados y
  las conclusiones, que revisamos y ajustamos.
- **Git**: la IA nos guió para resolver un conflicto de merge, configurar el `.gitignore`
  y separar el proyecto en este repositorio conservando los commits de los dos integrantes.

### 2.6 OpenCode

OpenCode se usó para descargar las canciones y convertirlas a WAV mono, 22 050 Hz,
16 bits, y para diagnosticar el fallo de lectura de la microSD (ver 2.1).

## 3. Cómo se verificó el código generado

- Todo se compiló y probó en la Nucleo-F401RE con el montaje real.
- Cada componente (programas de `pruebas/`) y cada función de la pantalla se probó por
  separado antes de integrarse.
- Se midió el tiempo de dibujo con la música sonando para comprobar que el doble buffer
  no se quedara sin datos.
- Las explicaciones de la IA se contrastaron con el manual de referencia (RM0368), la hoja
  de datos de la ST7789, la especificación de la SD en modo SPI y la documentación de FatFs.

## 4. Cómo se marca en el código

Cada archivo con ayuda de IA tiene al inicio un comentario que indica la modalidad:

```c
/* IA: generado por Claude; revisado, compilado y probado por nosotros. */
/* IA: estructura con ??? y pistas de Claude; completado y probado por nosotros. */
/* IA: escrito por nosotros con asesoría de Claude (correcciones puntuales). */
/* IA: hecho por nosotros; revisado y confirmado con Claude. */
```

Las partes puntuales dentro de un archivo (por ejemplo, el filtro pasa-altos en `main.c`)
llevan además un comentario `// IA:` en la línea donde empiezan.