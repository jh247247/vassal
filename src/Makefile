include ../Makefile.common

OBJS+=startup.o
OBJS+=main.o
OBJS+=lcd_control.o
OBJS+=timer.o
OBJS+=usart.o
OBJS+=json.o
OBJS+=gfx.o
OBJS+=util.o
OBJS+=bootloader.o


all: src

src: app.a

app.a: $(OBJS)
	$(AR) cr app.a $(OBJS)

.PHONY:src clean

clean:
	rm -f app.a $(OBJS)
