LUA_VERSION := 5.3.4
LUA_MAJOR_VER := 5
LUA_NAME := lua-$(LUA_VERSION)
LUA_DL := $(LUA_NAME).tar.gz
LUA_DL_URL := "https://www.lua.org/ftp/$(LUA_DL)"
LUA_SRC_DIR := $(PWD)/lua/$(LUA_NAME)/src
LUA_BUILD_DIR := $(PWD)/lua/$(LUA_NAME)/build

LUA_INCLUDE_DIR := $(GBBOII_INCLUDE_DIR)/lua
LUA_LIBDIR := $(GBBOII_LIB_DIR)

LUA_CFLAGS := -I$(LUA_INCLUDE_DIR)
LUA_LDFLAGS := -L$(LUA_LIBDIR) -llua

LUA_BUILD_LDFLAGS ?= -pthread
LUA_BUILD_CFLAGS ?= -Wall -std=gnu99

# Hacks to only download/build lua once.
LUA_LIB_BUILT 			 := $(LUA_SRC_DIR)/build_state
LUA_SRC_DIR_UNPACKED := $(LUA_SRC_DIR)/unpack_state
LUA_INCLUDE_DIR_MADE := $(LUA_SRC_DIR)/include_dir_state

$(LUA_LIB_BUILT): $(LUA_SRC_DIR_UNPACKED) $(LUA_INCLUDE_DIR_MADE)
	cd lua/$(LUA_NAME) && make -e MYCFLAGS="$(LUA_BUILD_CFLAGS) -fPIC -DLUA_COMPAT_5_2 -DLUA_COMPAT_5_1" MYLDFLAGS="$(LUA_BUILD_LDFLAGS)" linux
	cd lua/$(LUA_NAME) && make -e TO_LIB="liblua.a" INSTALL_DATA='cp -d' INSTALL_TOP='$(LUA_BUILD_DIR)' install
	cp $(LUA_BUILD_DIR)/bin/* $(PWD)/bin/
	cp $(LUA_BUILD_DIR)/include/*.h $(LUA_INCLUDE_DIR)
	cp $(LUA_BUILD_DIR)/lib/*.a $(LUA_LIBDIR)
	echo 1 > $(LUA_LIB_BUILT)

$(LUA_SRC_DIR_UNPACKED):
	wget $(LUA_DL_URL)
	tar xf $(LUA_DL)
	$(RM) $(LUA_DL)
	mv $(LUA_NAME) lua/
	cd lua/$(LUA_NAME) && patch -p1 -i ../lua.patch
	echo 1 > $(LUA_SRC_DIR_UNPACKED)

$(LUA_INCLUDE_DIR_MADE):
	mkdir -p $(LUA_INCLUDE_DIR)
	echo 1 > $(LUA_INCLUDE_DIR_MADE)

lua_clean:
	cd $(LUA_SRC_DIR) && make clean

lua_fullclean: lua_clean
	$(RM) -r lua/$(LUA_NAME)
	$(RM) -r $(LUA_BUILD_DIR)
	$(RM) -r $(LUA_INCLUDE_DIR)
	$(RM) $(LUA_LIBDIR)/*lua*
