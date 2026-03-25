LFLAGS=-Wall -funroll-loops -fomit-frame-pointer -pipe -O9
CFLAGS=-Wall -funroll-loops -fomit-frame-pointer -pipe -O9 
CC=gcc
GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0 2>/dev/null)
GLIB_LFLAGS := $(shell pkg-config --libs glib-2.0 2>/dev/null)
ifeq ($(strip $(GLIB_LFLAGS)),)
GLIB_LFLAGS := -lglib-2.0
endif
#
# Build with SDL2 + SDL2_mixer on modern distros.
# (The upstream project originally used SDL 1.x.)
#
SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL2_LIBS   := $(shell sdl2-config --libs 2>/dev/null)
SDL2_MIXER_CFLAGS := $(shell pkg-config --cflags SDL2_mixer 2>/dev/null || pkg-config --cflags SDL_mixer 2>/dev/null)
SDL2_MIXER_LIBS := $(shell pkg-config --libs SDL2_mixer 2>/dev/null || pkg-config --libs SDL_mixer 2>/dev/null || echo -lSDL2_mixer -lSDL2)

NCURSES_LIBS := $(shell pkg-config --libs ncursesw 2>/dev/null || echo -lncurses)

all: nog_srv nog_ncurses nog_ncurses_nosound

nog_srv: debug.o nog_srv.o network.o engine.o dlb.o linked.o
	$(CC) $(CFLAGS) -o ../nog_srv nog_srv.o debug.o network.o engine.o dlb.o linked.o $(GLIB_LFLAGS)

nog_ncurses : debug.o client.o nog_ncurses.o network.o linked.o dlb.o cli_ncurses.o engine.o sound.o nog_ncurses.o
	$(CC) $(CFLAGS) -o ../nog_ncurses nog_ncurses.o debug.o network.o linked.o dlb.o cli_ncurses.o engine.o client.o sound.o $(SDL2_LIBS) $(SDL2_MIXER_LIBS) $(GLIB_LFLAGS) $(NCURSES_LIBS)

nog_ncurses_nosound : debug.o client_nosound.o nog_ncurses_nosound.o network.o linked.o dlb.o cli_ncurses.o engine.o nog_ncurses_nosound.o
	$(CC) $(CFLAGS) -o ../nog_ncurses_nosound nog_ncurses_nosound.o debug.o network.o linked.o dlb.o cli_ncurses.o engine.o client_nosound.o $(GLIB_LFLAGS) $(NCURSES_LIBS)


nog_srv.o : nog_srv.c
	gcc $(LFLAGS) $(GLIB_CFLAGS) -c nog_srv.c

client.o: client.c
	gcc $(LFLAGS) $(SDL2_CFLAGS) $(SDL2_MIXER_CFLAGS) -c client.c

client_nosound.o: client_nosound.c
	gcc $(LFLAGS) -c client_nosound.c

nog_ncurses.o: nog_ncurses.c
	gcc $(LFLAGS) $(GLIB_CFLAGS) $(SDL2_CFLAGS) $(SDL2_MIXER_CFLAGS) -c nog_ncurses.c

nog_ncurses_nosound.o: nog_ncurses_nosound.c
	gcc $(LFLAGS) $(GLIB_CFLAGS) -c nog_ncurses_nosound.c

nog_sdl.o: nog_sdl.c
	gcc $(LFLAGS) $(GLIB_CFLAGS) -c nog_sdl.c


debug.o: debug.c
	gcc $(LFLAGS) -c debug.c
	
network.o: network.c 
	gcc $(LFLAGS) -c network.c

engine.o : engine.c 
	gcc $(LFLAGS) -c engine.c

dlb.o:	dlb.c dlb.h
	gcc $(LFLAGS) -c dlb.c

linked.o: linked.c 
	gcc $(LFLAGS) -c linked.c

cli_ncurses.o: cli_ncurses.c 
	gcc $(LFLAGS) -c cli_ncurses.c

cli_sdl.o: cli_sdl.c 
	gcc $(LFLAGS) -c cli_sdl.c

sprite.o: sprite.c
	gcc $(LFLAGS) -c sprite.c 

sound.o: sound.c
	gcc $(LFLAGS) $(SDL2_CFLAGS) $(SDL2_MIXER_CFLAGS) -c sound.c
	

clean:
	-rm -f *.o nog_srv nog_ncurses *.log
	-rm -rf bcpp

bcpp:
	rm -rf bpcc
	mkdir bpcc
	cd bcpp || exit
	cp ../*.h ../*.c .
	for i in *; do bcpp $i > ../$i; done

