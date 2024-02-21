# Compiler options
WARNINGS := -std=c99 -Werror -Wall -Wextra
DEV_WARNINGS := -Wconversion -Wshadow -Wpedantic -pedantic-errors -Wformat=2
MACROS := -DFRYZ
DEV_MACROS := -DFRYZ_DEBUG -DFRYZ_HOT_RELOAD -DLIBFRYZ_PATH=\"$(shell realpath ./dist/libfryz.so)\"
SHARED_FLAGS := -shared -fPIC
DEBUG_FLAGS := -ggdb3 -fPIC
LIBRARIES := raylib
FLAGS := $(shell pkg-config --cflags $(LIBRARIES))
LINKS := $(shell pkg-config --libs $(LIBRARIES))

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


ifeq ($(filter dev, $(MAKECMDGOALS)), dev)
	WARNINGS += $(DEV_WARNINGS)
	MACROS += $(DEV_MACROS)
	FLAGS += $(DEBUG_FLAGS)
else ifeq ($(filter reload, $(MAKECMDGOALS)), reload)
	WARNINGS += $(DEV_WARNINGS)
	MACROS += $(DEV_MACROS)
	FLAGS += $(DEBUG_FLAGS)
else
	OBJECTS += $(LIBFRYZ_OBJECTS)
	SHARED_FLAGS :=
endif

.PHONY: all clean dev prod reload

prod: all

dev: $(LIBFRYZ) all

all: $(DIST)/fryz $(HEADERS) Makefile | $(DIST)

reload: $(LIBFRYZ)

$(DIST)/fryz: $(OBJECTS) $(HEADERS)
	cc -o $@ $(OBJECTS) $(WARNINGS) $(MACROS) $(LINKS) $(FLAGS)

$(LIBFRYZ): $(LIBFRYZ_OBJECTS)
	cc -o $@ $^ $(WARNINGS) $(MACROS) $(SHARED_FLAGS) $(LINKS) $(FLAGS)

$(DIST)/%.o: $(LIBFRYZ_SRC)/%.c
	cc -o $@ -c $< $(WARNINGS) $(MACROS) $(LINKS) $(FLAGS)

$(DIST)/%.o: $(SRC)/%.c
	cc -o $@ -c $< $(WARNINGS) $(MACROS) $(LINKS) $(FLAGS)

clean:
	rm $(DIST)/*
