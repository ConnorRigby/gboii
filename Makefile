GBBOII_VERSION_MAJOR := 0
GBBOII_VERSION_MINOR := 1
GBBOII_VERSION_PATCH := 0

GBBOII_LIB_NAME := libgbboii

GBBOII_LIB_DIR := lib
GBBOII_INCLUDE_DIR := include

GBBOII_SRC := $(wildcard src/gbboii/*.cpp)
GBBOII_HEADERS := $(wildcard include/gbboii/*.hpp)
GBBOII_OBJ := $(GBBOII_SRC:.cpp=.o)

CFLAGS := -Wall -Wextra -std=c++17 -I$(GBBOII_INCLUDE_DIR) -DDEBUG -g
LDFLAGS :=

SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LDFLAGS := $(shell pkg-config --libs sdl2)

GBBOII_LIB_CFLAGS := $(CFLAGS) -fPIC
GBBOII_LIB_LDFLAGS := $(LDFLAGS) -fPIC --shared -lc -Wl,-soname,$(GBBOII_LIB_NAME).so.$(GBBOII_VERSION_MAJOR).$(GBBOII_VERSION_MINOR).$(GBBOII_VERSION_PATCH)

GBBOII_LIB := $(GBBOII_LIB_DIR)/$(GBBOII_LIB_NAME).so.$(GBBOII_VERSION_MAJOR).$(GBBOII_VERSION_MINOR).$(GBBOII_VERSION_PATCH)

include lua/lua.Makefile

.PHONY: all clean run lua_clean lua_fullclean
.DEFAULT_GOAL := all

all: $(GBBOII_LIB)

clean:
	$(RM) $(GBBOII_OBJ)
	$(RM) $(GBBOII_LIB)
	$(RM) $(GBBOII_LIB_DIR)/$(GBBOII_LIB_NAME).so.$(GBBOII_VERSION_MAJOR)
	$(RM) $(GBBOII_LIB_DIR)/$(GBBOII_LIB_NAME).so
	$(RM) bin/main

$(GBBOII_LIB): $(LUA_LIB_BUILT) $(GBBOII_SRC) $(GBBOII_HEADERS) $(GBBOII_OBJ)
	$(CXX) $(GBBOII_LIB_CFLAGS) $(LUA_CFLAGS) $(GBBOII_LIB_LDFLAGS) $(GBBOII_OBJ) -llua -o $@
	ln -s --force $@ $(GBBOII_LIB_DIR)/$(GBBOII_LIB_NAME).so.$(GBBOII_VERSION_MAJOR)
	ln -s --force $@ $(GBBOII_LIB_DIR)/$(GBBOII_LIB_NAME).so

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $(LUA_CFLAGS) -fPIC $(LDFLAGS) $(LUA_LDFLAGS) -o $@ $<

bin/main: src/main.cpp src/gpu/*.cpp src/gpu/*.hpp
	$(CXX) $(CFLAGS) $(SDL_CFLAGS) $(LDFLAGS) $(SDL_LDFLAGS) -llua $(GBBOII_LIB_DIR)/libgbboii.so.0.1.0 -o $@ src/main.cpp

run: all bin/main
	LD_LIBRARY_PATH=$(GBBOII_LIB_DIR) bin/main DMG_ROM.bin test.lua
