TEMPLATEROOT =/l/arm/STM32-Template

# compilation flags for gdb

CFLAGS += -g -I./Library 
CFLAGS += -g -I./Library/ff9/src 
ASFLAGS += -g
LDLIBS += -lm

# project files

vpath %.c ./Library/ff9/src
vpath %.c ./Library/ff9/src/option
vpath %.c ./Library/

# object files

OBJS=  $(STARTUP) main.o spidma.o lcdma.o i2c.o audiodma.o
OBJS +=	ff.o mmcbb.o
OBJS+=  stm32f10x_gpio.o stm32f10x_rcc.o stm32f10x_spi.o stm32f10x_dma.o stm32f10x_i2c.o stm32f10x_dac.o stm32f10x_tim.o misc.o

# include common make file

include $(TEMPLATEROOT)/Makefile.common


