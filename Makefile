# Compiler options
WARNINGS := -std=c99 -Werror -Wall -Wextra -Wno-format-truncation
DEV_WARNINGS := -Wshadow -Wpedantic -pedantic-errors -Wformat=2 #-Wconversion
MACROS := -DFRYZ -DLIBFRYZ_PATH=\"$(shell echo "${PWD}/dist/libfryz.so")\" -DFRYZ_FONT_PATH=\"$(shell realpath ./resources/RobotoMono-Regular.ttf)\" -DDEFAULT_AUDIO_PATH=\"$(shell realpath ./resources/audio/Glory.wav)\" 
DEV_MACROS := -DFRYZ_DEBUG -DFRYZ_HOT_RELOAD
SHARED_FLAGS := -shared -fPIC
DEBUG_FLAGS := -ggdb3 -fPIC
LIBRARIES := raylib fftw3f
FLAGS := $(shell pkg-config --cflags $(LIBRARIES))
LINKS := $(shell pkg-config --libs $(LIBRARIES)) -lm

# Directories, Files
DIST := ./dist
$(DIST):
	mkdir $(DIST)
SRC := ./src
LIBFRYZ_SRC := $(SRC)/libfryz
INLCUDE := ./src/include
HEADERS := $(wildcard $(INCLUDE)/*.h)
SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(DIST)/%.o, $(SOURCES))
LIBFRYZ_SOURCES := $(wildcard $(LIBFRYZ_SRC)/*.c)
LIBFRYZ_OBJECTS := $(patsubst $(LIBFRYZ_SRC)/%.c, $(DIST)/%.o, $(LIBFRYZ_SOURCES))
LIBFRYZ := $(DIST)/libfryz.so

# Change variables depeing on if this a development build or
# production build
ifeq ($(filter dev, $(MAKECMDGOALS)), dev)
	WARNINGS += $(DEV_WARNINGS)
	MACROS += $(DEV_MACROS)
	FLAGS += $(DEBUG_FLAGS)
else ifeq ($(filter reload, $(MAKECMDGOALS)), reload)
	WARNINGS += $(DEV_WARNINGS)
	MACROS += $(DEV_MACROS)
	FLAGS += $(DEBUG_FLAGS)
else # all, prod
# Essentially treats libryz objects like regular objects
# so that they can be linked normally
	OBJECTS += $(LIBFRYZ_OBJECTS)
	SHARED_FLAGS :=
endif

# Targets
.PHONY: all clean dev prod reload

prod: all

dev: $(LIBFRYZ) all

all: $(DIST)/fryz $(HEADERS) Makefile | $(DIST)

reload: $(LIBFRYZ)

$(DIST)/fryz: $(OBJECTS) $(HEADERS)
	gcc -o $@ $(OBJECTS) $(WARNINGS) $(MACROS) $(LINKS) $(FLAGS)

$(LIBFRYZ): $(LIBFRYZ_OBJECTS) $(HEADERS)
	gcc -o $@ $(LIBFRYZ_OBJECTS) $(WARNINGS) $(MACROS) $(SHARED_FLAGS) $(LINKS) $(FLAGS)
	chmod -x $(LIBFRYZ)	# Not sure why it produces an executable (all it does is segfault)

$(DIST)/%.o: $(LIBFRYZ_SRC)/%.c
	gcc -o $@ -c $< $(WARNINGS) $(MACROS) $(LINKS) $(FLAGS)

$(DIST)/%.o: $(SRC)/%.c
	gcc -o $@ -c $< $(WARNINGS) $(MACROS) $(LINKS) $(FLAGS)

clean:
	rm $(DIST)/*
