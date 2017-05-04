//
// Created by Mark Plagge on 4/28/17.
//

/**
  This file contains functions used to parse the text model configuration.
 */

#include "IOStack.h"
#include <search.h>
#include "../lib/simclist.h"
#include "../neuro/tn_neuron.h"
#include "../lib/csv.h"
/* Input uses standard lua 5.1. However, these includes may be switched out
 * with luaJIT in the future. Maintain lua 5.1 / partial 5.2 compatiblility. */
#include "../lib/lua/lua.h"
#include "../lib/lua/lualib.h"
#include "../lib/lua/lauxlib.h"


/**
 * L -> Global (to the model def) state of the lua file
 */
lua_State *L;

//!! Global LUA helper function names.
char* loadCFGfn = "loadFile";
char* getNParfn = "getNeuronParam";
char* nExistfn = "doesNeuronExist";
char* luaUtilFile = "./model_read.lua";

long curCoreID, curLocalID;
char * curType;
unsigned  long maxNeurons;

int countLines(FILE *fileHandle){
	int ch;
	int charsOnCurrentLine = 0;
	int count = 0;
	while ((ch = fgetc(fileHandle)) != EOF) {
		if (ch == '\n') {
			count++;
			charsOnCurrentLine = 0;
		} else {
			charsOnCurrentLine++;
		}
	}
	if (charsOnCurrentLine > 0) {
		count++;
	}
	return count;
}

void initModelInput(unsigned long maxNeurons){
	L = luaL_newstate();
	luaL_openlibs(L);
	int s = luaL_loadfile(L, NETWORK_CFG_FN);

	if(!s)
		s = lua_pcall(L, 0, LUA_MULTRET, 0);

	//show any errors
	if(s){
		printf("Error: %s \n", lua_tostring(L, -1));
		tw_error(TW_LOC,"MDL_LOAD", "Unable to load config file %s \n", NETWORK_CFG_FN );

		lua_pop(L, 1);
	}
	s = luaL_loadfile(L, luaUtilFile);

	if(!s)
		s = lua_pcall(L, 0, LUA_MULTRET, 0);

	//show any errors
	if(s){
		printf("Error: %s \n", lua_tostring(L, -1));
		lua_pop(L, 1);
		tw_error(TW_LOC,"MDL_LOAD", "Unable to load helper file %s \n", luaUtilFile );

	}
}

void lPushParam(char* paramName){
	lua_getglobal(L, getNParfn);
	lua_pushnumber(L, curCoreID);
	lua_pushnumber(L, curLocalID);
	lua_pushstring(L, curType);
	lua_pushstring(L, paramName);
	lua_call(L, 4, 1);
}


long getLuaArray(long * arr){

	lua_pushvalue(L, -1);
	lua_pushnil(L);
	int elnum = 0;
	while ( lua_next(L, -2) != 0){
		lua_pushvalue(L, -2);

		const char *key = lua_tostring(L, -1);
		long value = lua_tointeger(L, -2);
		arr[elnum] = value;
		elnum ++;
		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	return elnum - 1;
}

long lGetParam(int isArray, long * arrayParam){

	if(isArray){
		return getLuaArray(arrayParam);
	}else{
		long value = lua_tointeger(L, -1);
		return value;
	}

	return -1;
}


//check to see if a neuron exists in the config file
bool neuronExists(){
	lua_getglobal(L, "doesNeuronExist");
	lua_pushnumber(L, curCoreID);
	lua_pushnumber(L, curLocalID);
	lua_pushstring(L, curType);
	lua_call(L, 3, 1);
	return (lua_toboolean(L, -1) == true);

}

int lookupAndPrimeNeuron(long coreID, long localID, char * nt){
	curCoreID = coreID;
	curLocalID = localID;
	curType = nt;

	if (neuronExists()){
		if (DBG_MODEL_MSGS){
			printf("NeuronExists - %li_%s_%li \n", coreID, localID, nt);
		}
		return 0;
	}else{
		curCoreID = 0;
		curLocalID = 0;
		curType = "";
	}
	return -1;
}

long lGetAndPushParam(char * paramName, int isArray, long * arrayParam){
	lPushParam(paramName);
	return lGetParam(isArray,arrayParam);
}

//enum TNReadMode{
//    CONN , //Syn. Connectivity
//    AXTP, //Axon Types
//    SGI, //sigma GI vals
//    SP, //S Vals
//    BV, //b vals
//    NEXT, //goto next array data chunk
//    OUT //out of array data
//};


static enum modelReadMode fileReadState = START_READ;
//static enum TNReadMode tnReadState = CONN;




