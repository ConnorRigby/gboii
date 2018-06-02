#include <gbboii/gbboii.hpp>
#include <gbboii/memory.hpp>
#include <gbboii/utils.hpp>
#include <gbboii/lua_lib.hpp>

extern "C"{
  #include <lua/lua.h>
  #include <lua/lauxlib.h>
  #include <lua/lualib.h>
  #include <lua/lualib.h>
}

static int l_gbboii_peek8(lua_State* L) {
  lua_getglobal(L, "__GBBOII__");
  void *user_data_ptr = lua_touserdata(L, -1);
  if(user_data_ptr == NULL) {
    debug_print("Could not find global gameboy object!\r\n");
    return 0;
  }
  Gameboy* gb = (Gameboy*)user_data_ptr;

  int r = luaL_checkinteger(L, 1);
  if(r >= MEMORY_SIZE) {
    luaL_error(L, "Argument to gbboii_peek8() out of bounds: %p\r\n", r);
  }

  uint16_t addr = (uint16_t)r;
  uint8_t data = gb->mem.mem[addr];
  lua_pushinteger(L, (int)data);
  return 1;
}

static int l_gbboii_poke8(lua_State* L) {
  lua_getglobal(L, "__GBBOII__");
  void *user_data_ptr = lua_touserdata(L, -1);
  if(user_data_ptr == NULL) {
    luaL_error(L, "Could not find global gameboy object!\r\n");
  }
  Gameboy* gb = (Gameboy*)user_data_ptr;

  int r;
  r = luaL_checkinteger(L, 1);
  if(r >= MEMORY_SIZE) {
    luaL_error(L, "First argument to gbboii_poke8() out of bounds: %p\r\n", r);
  }

  uint16_t addr = (uint16_t)r;

  r = luaL_checkinteger(L, 2);
  if(r >= BYTE_MAX) {
    luaL_error(L, "Second argument to gbboii_poke8() out of bounds: %p\r\n", r);
  }

  uint8_t data = (uint8_t)r;
  gb->mem.mem[addr] = data;
  return 0;
}

static int l_gbboii_print(lua_State* L) {
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; ++i) {
      debug_print_q(lua_tostring(L, i));
      debug_print_q(" ");
    }
    debug_print_q("\r\n");
    return 0;
}

static const struct luaL_Reg GBBOII_LUA_LIB [] = {
  {"print", l_gbboii_print},
  {"gbboii_peek8", l_gbboii_peek8},
  {"gbboii_poke8", l_gbboii_poke8},
  {NULL, NULL}
};

void Gameboy::lua_init() {
  // Store the lua state.
  L = luaL_newstate();

  // Push the pointer to this GB object onto the stack and store it as
  // the global name __GBBOII__
  lua_pushlightuserdata(L, (void*)this);
  lua_setglobal(L, "__GBBOII__");

  // Open the lib, get the global variable _G
  luaL_openlibs(L);
  lua_getglobal(L, "_G");

  // Inject functions and pop _G
  luaL_setfuncs(L, GBBOII_LUA_LIB, 0);
  lua_pop(L, 1);
}

int Gameboy::load_script(const char* filename) {
  int r = luaL_loadfile(L, filename);
  if(r == LUA_OK)  {
    debug_print("Successfully loaded %s\r\n", filename);
    if(lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
      debug_print_q("Error in script: \r\n\t%s\r\n", lua_tostring(L, -1));
    }
  } else {
    debug_print("Failed to load %s\r\n", filename);
  }
  return r;
}
