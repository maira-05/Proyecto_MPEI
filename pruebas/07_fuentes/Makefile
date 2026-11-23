#
# Makefile de la PRUEBA DE PANTALLA
# Se ejecuta desde la carpeta pruebas/07_fuentes/  ->  make   y   make flash
# Usa los drivers del proyecto (../../src) sin modificarlos y NO compila
# main.c, sd.c, audio.c, uart.c ni FatFs.
#
APPNAME = prueba

CC = arm-none-eabi-gcc
ARCH = -mcpu=cortex-m4
LINKERSCRIPT = ../../linker_script.ld

# Solo lo que necesita la pantalla
C_SRC  = prueba_fuentes.c tft_text.c
C_SRC += ../../src/system.c ../../src/gpio.c ../../src/spi.c ../../src/tft.c
ASM_SRC = ../../crt0.S

# Encabezados: los del proyecto + esta carpeta (fuentes y Adafruit_GFX.h)
INCLUDE  = -I. -I../../inc -I../../inc/cmsis

CFLAGS  = -Wall -Wextra -std=c11 -ffreestanding
CFLAGS += -Og -g -fno-pic
CFLAGS += $(ARCH) --specs=nosys.specs --specs=nano.specs
CFLAGS += $(INCLUDE) -DSTM32F401xE

LDFLAGS  = -T$(LINKERSCRIPT) -nostdlib -Wl,--no-relax
LDLIBS   = -lc -lgcc

# Los .o se guardan en build/ para no mezclarse con los del proyecto
OBJS  = $(addprefix build/, $(notdir $(C_SRC:.c=.o)))
OBJS += $(addprefix build/, $(notdir $(ASM_SRC:.S=.o)))
vpath %.c . ../../src
vpath %.S ../..

all: $(APPNAME).elf

$(APPNAME).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	arm-none-eabi-size -A -x $@

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.S | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

.PHONY: clean flash
clean:
	rm -rf build $(APPNAME).elf

flash: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"
