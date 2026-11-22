# Reproductor de música WAV con pantalla TFT — STM32F401RE (bare-metal)

Proyecto 4 de **Microprocesadores e Interfaces (MPEI) 2026** — Escuela Colombiana de Ingeniería Julio Garavito.

**Autores:** Jaime Galvis y Maira Prieto\
**Profesor:** Juan David Guerrero Balaguera

Reproductor de archivos WAV que lee 40 canciones de una microSD (FAT32), las reproduce
con un DAC por PWM y muestra el nombre de la canción, el estado y un menú en una pantalla
TFT ST7789 de 240×320. Se controla con cinco botones: Pause, Stop, Next, Prev y Menu.

Todo el firmware está escrito **sin HAL, LL ni CubeMX**: los drivers de los periféricos
trabajan directamente con los registros, y el arranque (`crt0.S`), el linker script y el
Makefile también se hicieron sin herramientas de ST (ver [USO_DE_IA.md](USO_DE_IA.md)). Se compila con la cadena GNU
(`arm-none-eabi-gcc` y `make`) y se graba con OpenOCD.

---

## Contenido

1. [Estructura del repositorio](#1-estructura-del-repositorio)
2. [Materiales](#2-materiales)
3. [Conexiones](#3-conexiones)
4. [Instalar las herramientas](#4-instalar-las-herramientas)
5. [Compilación y programación de la placa](#5-compilación-y-programación-de-la-placa)
6. [Preparar la microSD](#6-preparar-la-microsd)
7. [Uso](#7-uso)
8. [Pruebas por componente](#8-pruebas-por-componente)
9. [Problemas comunes](#9-problemas-comunes)
10. [Librerías externas y créditos](#10-librerías-externas-y-créditos)
11. [Uso de inteligencia artificial](#11-uso-de-inteligencia-artificial)

---

## 1. Estructura del repositorio

```
.
├── main.c               Lógica del reproductor: estados, botones, doble buffer, menú
├── crt0.S               Arranque: tabla de vectores, copia .data, limpia .bss, llama a main
├── linker_script.ld     Mapa de memoria: Flash 512 KB en 0x08000000, RAM 96 KB en 0x20000000
├── Makefile             Compilación con arm-none-eabi-gcc y grabación con OpenOCD
├── inc/                 Encabezados propios (uno por cada driver de src/)
│   ├── system.h         delay_ms y arranque del reloj
│   ├── gpio.h           Estructura GPIO_InitTypeDef, modos y pull-up/pull-down
│   ├── uart.h           uart_init, uart_enviar_texto, uart_numero
│   ├── spi.h            Inicio y transferencia de SPI1 y SPI2
│   ├── sd.h             Inicio de la microSD y lectura de sectores
│   ├── audio.h          audio_pwm_init, audio_muestreo_init, audio_poner_muestra
│   ├── tft.h            Funciones de la ST7789, pines, tamaño y colores RGB565
│   ├── pantalla.h       pantalla_inicio, pantalla_cancion, pantalla_estado, menú
│   └── cmsis/           Encabezados CMSIS (nombres de los registros del STM32F401)
├── src/                 Drivers propios
│   ├── system.c         Reloj a 84 MHz y delay_ms
│   ├── gpio.c           Configuración, lectura y escritura de pines
│   ├── uart.c           USART2 para depuración (115200 baudios)
│   ├── spi.c            SPI1 (microSD) y SPI2 (pantalla)
│   ├── sd.c             Protocolo de la microSD por SPI (CMD0, CMD8, ACMD41, CMD58, CMD17)
│   ├── audio.c          TIM2 = PWM en PA0 (~328 kHz); TIM3 = interrupción a 22 050 Hz
│   ├── tft.c            Driver de la ST7789: inicio, ventana, píxel, rectángulo
│   └── pantalla.c       Interfaz: pantalla de reproducción y menú
├── source/              FatFs (elm-chan) — sistema de archivos FAT32
│   ├── ff.c / ff.h      Núcleo de FatFs (f_mount, f_open, f_read, f_gets...)
│   ├── ffconf.h         Configuración de FatFs (FF_USE_STRFUNC = 1 para f_gets)
│   ├── ffsystem.c       Funciones de sistema de FatFs
│   ├── ffunicode.c      Tablas de caracteres de FatFs
│   ├── diskio.h         Interfaz que FatFs espera del disco
│   └── diskio.c         Puente entre FatFs y sd.c (escrito por nosotros)
├── tft_fuentes/         Dibujo de texto
│   ├── tft_text.c/.h    Dibuja letras con fuentes en formato GFXfont (propio)
│   ├── gfxfont.h        Estructuras de las fuentes
│   ├── Adafruit_GFX.h   Archivo puente para usar las fuentes sin Arduino
│   └── Fonts/           Fuentes de Adafruit GFX (FreeSansBold 9/12/18 pt, DigitalWF 24 pt)
├── pruebas/             Programas de prueba por módulo (ver sección 8)
├── sd/                  NOMBRES.TXT de ejemplo para copiar a la microSD
├── docs/                Esquemático (imagen y archivo de EasyEDA)
├── readme.md            Este archivo
├── USO_DE_IA.md         Declaración detallada del uso de IA
└── .gitignore           Excluye los archivos generados (.o, .elf, .bin, .map)
```

## 2. Materiales

| Cantidad | Componente            | Notas                                                         |
| -------- | --------------------- | ------------------------------------------------------------- |
| 1        | Nucleo-F401RE         | STM32F401RE (Cortex-M4) con ST-Link integrado                 |
| 1        | Módulo lector microSD | SIG0104 (SPI), alimentado a 5 V                               |
| 1        | microSD               | 32 GB, formateada en FAT32                                    |
| 1        | Pantalla              | TFT ST7789 240×320 (SPI), alimentada a 3,3 V                  |
| 1        | Amplificador          | Módulo GF1002 (PAM8403) con potenciómetro, fuente externa 5 V |
| 1        | Parlante              | 8 Ω, 10 W                                                     |
| 1        | Filtro RC pasa-bajos  | R = 1 kΩ, C = 10 nF (código 103) → f<sub>c</sub> ≈ 15,9 kHz   |
| 5        | Pulsadores            | Pause, Stop, Next, Prev, Menu                                 |
| —        | Cables y protoboard   |                                                               |

## 3. Conexiones

### Esquemático

![Esquemático del reproductor](docs/esquematico.png)

Archivo editable (EasyEDA): [`docs/esquematico.json`](docs/esquematico.json)

### Pines

| Periférico                       | Señal             | Pin del micro     |
| -------------------------------- | ----------------- | ----------------- |
| microSD (SPI1)                   | SCK / MISO / MOSI | PA5 / PA6 / PA7   |
|                                  | CS                | PB6               |
| Pantalla ST7789 (SPI2)           | SCK / SDA         | PB13 / PB15       |
|                                  | CS / DC / RST     | PB12 / PB14 / PB1 |
| Audio (TIM2 CH1)                 | PWM               | PA0               |
| Botones (a GND, pull-up interno) | Pause / Stop      | PC10 / PC11       |
|                                  | Next / Prev       | PC12 / PC8        |
|                                  | Menu              | PC6               |
| Encender / apagar                | Botón azul        | PC13              |
| Depuración (USART2)              | TX / RX           | PA2 / PA3         |

### Audio

PA0 → filtro RC pasa-bajos (R1 = 1 kΩ, C1 = 10 nF, código 103) → entrada R del módulo GF1002
(amplificador PAM8403) → parlante.

- **Filtro RC:** frecuencia de corte f<sub>c</sub> = 1 / (2π·R·C) ≈ 15,9 kHz. Deja pasar
  el audio (hasta ~11 kHz) y atenúa la portadora del PWM (~328 kHz).
- **Salida en puente:** el parlante va entre `R_OUT+` y `R_OUT−`. Ninguna de las dos
  salidas se conecta a GND: el PAM8403 maneja cada terminal con una señal opuesta.
- **Volumen:** se ajusta en dos puntos. En el firmware, cada muestra se desplaza
  `VOLUMEN_SHIFT` bits a la derecha antes de enviarla al PWM (con 1 bit la amplitud
  queda a la mitad, unos −6 dB), para no saturar el amplificador. En el hardware, el
  potenciómetro del módulo GF1002 ajusta la ganancia final.

### Alimentación

- **Nucleo-F401RE:** por el USB del ST-Link, desde el computador.
- **microSD:** desde el pin de 5 V de la Nucleo.
- **Pantalla:** desde el pin de 3,3 V de la Nucleo.
- **Amplificador GF1002:** desde una **fuente externa de 5 V**. No se alimenta desde
  la Nucleo porque esta recibe su energía del USB del computador y la corriente no
  alcanza para el parlante (se producían reinicios).
- **Tierra común:** el GND de la fuente externa está unido al GND de la Nucleo; sin
  eso la señal de audio no tiene una referencia común.

## 4. Instalar las herramientas

El proyecto se compila con la **cadena GNU para ARM**, sin IDE. Se probó en
**Windows 11 + WSL (Ubuntu)**; en Linux nativo es igual, sin el paso de `usbipd`.

### En WSL / Ubuntu

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi make openocd git
```

Comprobar:

```bash
arm-none-eabi-gcc --version
openocd --version
```

### En Windows (solo si usan WSL): pasar el ST-Link a WSL

1. Instalar usbipd-win (en PowerShell): `winget install usbipd`
2. Conectar la Nucleo por USB y buscar su número de bus:
   ```powershell
   usbipd list
   ```
   Buscar la fila *ST-Link Debug, USB Mass Storage, USB Serial Device* (por ejemplo `1-2`).
3. **Solo la primera vez**, en PowerShell **como administrador**:
   ```powershell
   usbipd bind --busid 1-2
   ```
4. **Cada vez que conecten la placa** (sin administrador):
   ```powershell
   usbipd attach --wsl --busid 1-2
   ```
5. En WSL, comprobar que aparece: `lsusb` → `STMicroelectronics ST-LINK/V2.1`.

## 5. Compilación y programación de la placa

### Herramientas necesarias
- `arm-none-eabi-gcc` (cadena de herramientas para ARM)
- `make`
- `openocd`
- Si usas WSL, también `usbipd-win`, que permite exponer el ST-Link a Linux

### Obtener el código y compilarlo
```bash
git clone https://github.com/maira-05/Proyecto_MPEI.git
cd Proyecto_MPEI
make
```

### Programar la Nucleo
Con WSL, antes que nada hay que vincular el ST-Link desde PowerShell abierto como administrador:
```powershell
usbipd list
usbipd bind --busid <BUSID>      # solo la primera vez
usbipd attach --wsl --busid <BUSID>
```
Después, desde WSL:
```bash
make flash
```
Para eliminar los archivos generados en la compilación: `make clean`.

### Funcionamiento del Makefile
- Compila todos los `.c` ubicados en la carpeta raíz, `src/`, `source/` y `tft_fuentes/`,
  además de `crt0.S`, usando las opciones `-mcpu=cortex-m4 -std=c11 -ffreestanding -Og -g`.
- Los encabezados se buscan en `inc/`, `inc/cmsis/`, `source/` y `tft_fuentes/`.
- El enlazado usa `linker_script.ld` y `-nostdlib`, incorporando únicamente `libc` y `libgcc`.
- Al finalizar, `arm-none-eabi-size` imprime el tamaño de cada sección.
- `make flash` invoca OpenOCD con `interface/stlink.cfg` y `target/stm32f4x.cfg`.

### Consumo de memoria
| Región                                | Usado    | Total   | %      |
|-------------------------------------- |--------- |-------- |------- |
| Flash (`.text` + `.rodata` + `.data`) | 22 924 B | 512 KiB | 4,4 %  |
| RAM (`.data` + `.bss`)                | 10 932 B | 96 KiB  | 11,1 % |

- Gran parte de la RAM la consume el doble buffer de audio (2 × 4096 B).
- Casi todo `.rodata` corresponde a los bitmaps de las fuentes.
- `.data` se cuenta en ambas memorias: su valor inicial reside en Flash y
  `crt0.S` lo copia a RAM durante el arranque.

### Función del linker script
`linker_script.ld` describe las dos memorias del STM32F401RE y la ubicación de cada sección:

| Región | Inicio     | Tamaño  |
|------- |------------|---------|
| FLASH  | 0x08000000 | 512 KiB |
| RAM    | 0x20000000 | 96 KiB  |

- **`.text`** (Flash): inicia con la tabla de vectores (`.isr_vector`), obligatoriamente
  en 0x08000000, seguida del código. `KEEP` impide que el enlazador la elimine.
- **`.rodata`** (Flash): datos constantes y bitmaps de las fuentes.
- **`.data`** (`> RAM AT> FLASH`): variables con valor inicial. Residen en RAM
  (`_sdata`–`_edata`), pero su valor inicial se almacena en Flash (`_sidata`).
- **`.bss`** (RAM, `NOLOAD`): variables inicializadas en cero (`_sbss`–`_ebss`). No
  utiliza espacio en Flash.
- **Pila**: arranca en `_estack` = 0x20018000 (extremo final de la RAM) y crece hacia
  abajo. Un `ASSERT` detiene el enlazado si le quedan menos de 2 KiB disponibles.

Durante el arranque, `crt0.S` se apoya en estos símbolos para copiar `.data` de Flash
a RAM, poner `.bss` en cero y, por último, llamar a `main()`.

## 6. Preparar la microSD

1. Formatear la microSD en **FAT32**.
2. Crear la carpeta `MUSIC/EPIC/` en la raíz.
3. Convertir cada canción a **WAV mono, 22 050 Hz, 16 bits** y nombrarlas
   `CANC01.WAV` … `CANC40.WAV`. Con [FFmpeg](https://ffmpeg.org/):
   ```bash
   ffmpeg -i "cancion original.mp3" -ac 1 -ar 22050 -c:a pcm_s16le CANC01.WAV
   ```
   (`-ac 1` = mono, `-ar 22050` = frecuencia de muestreo, `pcm_s16le` = 16 bits con signo).
4. Copiar `NOMBRES.TXT` a la misma carpeta: **una línea por canción, en el mismo orden**
   que los archivos (línea 1 = CANC01). Solo caracteres ASCII, sin tildes ni ñ (la fuente
   no las tiene). Si el archivo no está, la pantalla muestra `CANC01`, `CANC02`...

Estructura final de la SD:

```
MUSIC/
└── EPIC/
    ├── CANC01.WAV
    ├── ...
    ├── CANC40.WAV
    └── NOMBRES.TXT
```

## 7. Uso

| Botón         | Pantalla de reproducción                               | Menú abierto                            |
| ------------- | ------------------------------------------------------ | --------------------------------------- |
| **Azul**      | Enciende / apaga                                       | Apaga                                   |
| **Menu**      | Abre el menú (pausa la música)                         | Sale sin cambiar de canción             |
| **Next**      | Siguiente canción (40 → 1)                             | Baja la selección                       |
| **Prev**      | Canción anterior (1 → 40)                              | Sube la selección                       |
| **Pause**     | Pausa / continúa (si está en Stop, empieza la canción) | Elige la canción marcada y la reproduce |
| **Stop**      | Detiene la canción                                     | Sale sin cambiar de canción             |

- Al encender se abre el menú para elegir la canción.
- Al terminar una canción pasa sola a la siguiente.
- Por el puerto serie (115200 baudios, 8N1) se ven mensajes de depuración: montaje de la
  SD, canción que se abre, botones presionados.

## 8. Pruebas por componente

Antes de integrar el reproductor, cada parte del hardware se probó por separado con un
programa independiente. Están en `pruebas/`, numerados en el orden en que se hicieron. Cada uno
es **independiente**: tiene su propio `Makefile`, `crt0.S` y linker script, y funciona con
el reloj interno de 16 MHz (HSI), sin depender de los drivers del proyecto.

Para correr cualquiera:

```bash
cd pruebas/01_pines       # o la prueba que se quiera
make
make flash
```

| Prueba                 | Qué comprueba                        | Si funciona                               |
| ---------------------- | ------------------------------------ | ----------------------------------------- |
| `01_pines`             | Cableado de la pantalla, sin SPI     | Cada pin marca 3,3 V durante 2 s          |
| `02_pantalla_parlante` | ST7789 en modo 0 (8 MHz) y audio PWM | Colores en pantalla y tonos 440–880 Hz    |
| `03_pantalla_modo3`    | ST7789 en modo 3 (1 MHz) y audio PWM | Igual que la 02 (esta sí funcionó)        |
| `04_sd`                | Inicio de la SD, sector 0 y FAT32    | No funcionó: se quedó en azul (ver notas) |
| `05_sd_uart`           | Lo mismo que la 04, por UART         | No funcionó (ver notas)                   |
| `06_audio_flash`       | PWM (TIM2) + TIM3 a 22 050 Hz        | Suena un fragmento guardado en Flash      |

**Notas:**

- **`01_pines`** pone en alto cada pin de la pantalla (CS, SCK, SDA, DC, RST) durante 2 s,
  uno por uno, para medirlo con el multímetro.
- **`02` y `03`** muestran rojo, verde, azul y blanco, y tocan 440, 660 y 880 Hz.
- **`02` frente a `03`:** la pantalla no funcionó en modo SPI 0; la versión en modo 3
  (CPOL = 1, CPHA = 1) sí, y ese es el modo que usa el driver del proyecto.
- **`04_sd` y `05_sd_uart` no funcionaron:** la 04 se quedaba en la pantalla azul de
  "cargando" (la SD nunca terminaba de iniciar; si falla en un paso, el LED parpadea N
  veces, donde N es ese paso). Para saber si el problema era el módulo y la tarjeta o el
  STM32, se probaron con un Arduino usando el ejemplo de la librería SD de Arduino, y ahí
  sí leyó: el fallo estaba del lado del STM32. En el proyecto final se corrigió moviendo la
  SD de PB3–PB5 a PA5–PA7 (PB3 es la salida SWO), subiendo la velocidad de los pines del
  SPI1 y agregando la ventana de voltaje en ACMD41. Estas dos pruebas usan el cableado
  viejo: PB3 (SCK), PB4 (MISO) y PB5 (MOSI).
- **`06_audio_flash`** no usa la microSD: el fragmento está en `src/cancion.c`. Para
  cambiarlo: `python wav_a_c.py CANCION.WAV [inicio_s] [duracion_s]` (máximo ~18 s).
- **Herramienta en el PC:** `pruebas/herramientas/revisar_wavs.py` revisa que los WAV
  de la microSD tengan el formato correcto (PCM, 16 bits, mono, 22 050 Hz, nombre 8.3):
  `python revisar_wavs.py E:\` (Windows) o `python revisar_wavs.py /media/sd` (Linux).

En el proyecto final, el driver de la SD (`sd.c`) imprime por UART la respuesta a cada
comando de inicio (CMD0, CMD8, ACMD41, CMD58), lo que sirve como modo de prueba del
driver propio.

## 9. Problemas comunes

| Síntoma                        | Causa                            | Solución                                        |
| ------------------------------ | -------------------------------- | ----------------------------------------------- |
| `make flash` → `open failed`   | ST-Link no conectado a WSL       | `usbipd attach --wsl --busid 1-2`               |
| `Device is not shared`         | Falta el `bind`                  | `usbipd bind --busid 1-2` (administrador)       |
| CMD0 no responde `01` (UART)   | Cableado de la SD o PB3 usado    | Revisar PA5–PA7 y PB6; no usar PB3 (SWO)        |
| La SD falla o lee mal bytes    | Pines del SPI1 en velocidad baja | Configurarlos en velocidad muy alta (`spi.c`)   |
| Colores invertidos en pantalla | Inversión activa en la ST7789    | `tft_init` envía INVOFF (0x20)                  |
| Sale `CANCxx` y no el nombre   | No encontró `NOMBRES.TXT`        | Debe estar en `MUSIC/EPIC/`                     |
| Audio saturado                 | Ganancia alta del amplificador   | Bajar el potenciómetro o subir `VOLUMEN_SHIFT`  |

## 10. Librerías externas y créditos

Los drivers de periféricos se escribieron para este proyecto (con apoyo de IA, ver
[USO_DE_IA.md](USO_DE_IA.md)). Las únicas librerías externas son:

- **FatFs** de ChaN (elm-chan) — `source/` (excepto `diskio.c`). Licencia propia tipo BSD.
  <http://elm-chan.org/fsw/ff/>
- **Fuentes de Adafruit GFX** (derivadas de GNU FreeFont) — `tft_fuentes/Fonts/`. Solo los
  datos de las fuentes; el dibujo (`tft_text.c`) es propio. Licencia BSD (Adafruit) y GPL
  con excepción de fuentes (FreeFont).
  <https://github.com/adafruit/Adafruit-GFX-Library>
- **CMSIS** (ARM / STMicroelectronics) — `inc/cmsis/`, solo encabezados de registros.

## 11. Uso de inteligencia artificial

Se usaron herramientas de IA durante el desarrollo. El detalle (qué herramienta, para qué,
qué código se generó y cómo se revisó y modificó) está en [USO_DE_IA.md](USO_DE_IA.md).