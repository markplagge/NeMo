//
// Created by Mark Plagge on 4/28/17.
//

/**
  This file contains functions used to parse the text model configuration.
 */

#include <sys/stat.h>
#include "IOStack.h"
#include "../extern/kdtree/kdtree.h"
/* Input uses standard lua 5.1. However, these includes may be switched out
 * with luaJIT in the future. Maintain lua 5.1 / partial 5.2 compatiblility. */
//#include "../extern/lua/lua.h"
//#include "../extern/lua/lualib.h"
//#include "../extern/lua/lauxlib.h"

#include "lua.h"
#include "../neuro/tn_neuron_struct.h"


/**@{ Binary Reader Code */
FILE *bin_file;
int bin_file_open = 0;
void *kd, *set;
tn_neuron_state **state_vec;
long num_n;

int openBinaryModelFile(char * binFileName){
  if(bin_file_open == 0){
    bin_file = fopen(binFileName,"rb");
    bin_file_open = 1;
  }
  return errno;
}

long setupBinaryNeurons(){
    //load metadata from file:
    long num_neurons;
    fscanf(bin_file,"%li\n",&num_neurons);
    state_vec = calloc(num_neurons, sizeof(tn_neuron_state*));
    //Load neurons into array of neurons:
    //tn_neuron_state **bin_neurons = tw_calloc(TW_LOC,"TEMP_KD",sizeof(tn_neuron_state*), num_neurons);
    //fread(bin_neurons,sizeof(bin_neurons[0]), num_neurons,bin_file);
    //loaded neurons. Store them in the KD tree:
    kd = kd_create(2);
    for(int i = 0; i < num_neurons; i ++){
        //tn_neuron_state *n = bin_neurons[i];
        tn_neuron_state *n = tw_calloc(TW_LOC,"Binary Neuron", sizeof(tn_neuron_state),1);
        state_vec[i] = n;
        fread(n,sizeof(n), 1,bin_file);
        int core_id = n->myCoreID;
        int local_id = n->myLocalID;
        float id[] = {core_id,local_id};
        kd_insertf(kd,id,n);
    }
    num_n = num_neurons;
    return num_neurons;
}

bool loadNeuronFromBIN(id_type neuronCore, id_type neuronLocal, tn_neuron_state *n){
    tn_neuron_state *old_state = n;
    float pos[] = {neuronCore,neuronLocal};
    struct kdres *res = kd_nearestf(kd,pos);
    tn_neuron_state *new_state;
    new_state = kd_res_itemf( res,pos);
    bool found = false;
    if(new_state->myLocalID == neuronLocal && new_state->myCoreID == neuronCore){
#ifdef DEBUG
        static int anc = 0;
        if(anc == 0) {
            tw_printf(TW_LOC, "Found Neuron.\n");
            anc = 1;
        }
#endif

        memcpy(n,new_state,sizeof(tn_neuron_state));
        found = true;
    }

    kd_res_free(res);
    return found;
}

void closeBinaryModelFile(){
    static data_free = 0;
    if (bin_file_open){
        fclose(bin_file);
    }
    //free kd tree...
    if(data_free == 0){
        data_free = 1;
        for(int i = 0; i < num_n; i ++){
            free(state_vec[i]);
        }
        free(state_vec);
        kd_free(kd);
    }


}
/**@}*/

/**
 * L -> Global (to the model def) state of the lua file
 */
lua_State *L;
lua_State *LT;

//!! Global LUA helper function names.
char *loadCFGfn = "loadFile";
char *getNParfn = "getNeuronParam";
char *nExistfn = "doesNeuronExist";
char *luaUtilFile = "model_read.lua";
char *tnLuaLU = "tn_types.lua";

long curCoreID, curLocalID;
char *curType;
unsigned long maxNeurons;


int countLines(FILE *fileHandle) {
  int ch;
  int charsOnCurrentLine = 0;
  int count = 0;
  while ((ch = fgetc(fileHandle))!=EOF) {
    if (ch=='\n') {
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

void initModelInput(unsigned long maxNeurons) {

  if (g_tw_mynode==0)
    printf("Model config file init\n");

  L = luaL_newstate();
  luaL_openlibs(L);

  int s;
  //s = luaL_loadfile(L, MODEL_FILE);
  if (isBin) {
    s = luaL_loadbuffer(L, luaConfigFile, isBin, "nc");
  } else {
    s = luaL_loadstring(L, luaConfigFile);
  }


/*  FILE *mdl_file;
  struct stat buf;
  stat(MODEL_FILE,&buf);
  off_t fsize = buf.st_size;
  FILE * fp = fopen(MODEL_FILE,"r");
  int fd = fileno(fp);

//  printf("malloc load \n");
//  char * fileCont = malloc(fsize +=1);
//  char c;
//  int ctr = 0;
//  while ((c = fgetc(fp)) != EOF){
//    fileCont[ctr ++] = c;
//  }

  //mmap fun
  char * modelFile = mmap(0,fsize+1,PROT_READ,MAP_PRIVATE,fd,0);
  printf("mapped file - loading model into lua parser.\n");


  s = luaL_dostring(L, modelFile);
  //s = luaL_dostring(L, fileCont);



  printf("lua model loaded. Unloading file.\n");
  munmap(modelFile, fsize+1);
  //free(fileCont);

  printf("model unloaded.\n");

*/


  if (g_tw_mynode==0)
    printf("File loaded - starting parsing...\n");
  if (!s)
    s = lua_pcall(L, 0, LUA_MULTRET, 0);

  //show any errors
  if (s) {
    printf("Error: %s \n", lua_tostring(L, -1));
    tw_error(TW_LOC, "MDL_LOAD", "Unable to parse config file  %s (from within LUA) \n", NEMO_MODEL_FILE_PATH);

    lua_pop(L, 1);
  }
  s = luaL_loadfile(L, luaUtilFile);

  if (!s)
    s = lua_pcall(L, 0, LUA_MULTRET, 0);

  //show any errors
  if (s) {
    printf("Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
    tw_error(TW_LOC, "MDL_LOAD", "Unable to load helper file %s \n", luaUtilFile);

  }
  LT = luaL_newstate();
  luaL_openlibs(LT);

  s = luaL_loadfile(LT, tnLuaLU);

  if (!s)
    s = lua_pcall(LT, 0, LUA_MULTRET, 0);

  //show any errors
  if (s) {
    printf("Error: %s \n", lua_tostring(L, -1));
    printf("fn: %s \n", tnLuaLU);
    tw_error(TW_LOC, "MDL_LOAD", "Unable to load LUA-> Config file %s \n", tnLuaLU);

    lua_pop(L, 1);
  }
  if (g_tw_mynode==0)
    printf("Parsing of configfile complete.");


}

void lPushParam(char *paramName) {
  lua_getglobal(L, getNParfn);
  lua_pushnumber(L, curCoreID);
  lua_pushnumber(L, curLocalID);
  lua_pushstring(L, curType);
  lua_pushstring(L, paramName);
  lua_call(L, 4, 1);
}

void getModelErrorInfo(int ncore, int nlocal, char *ntype, char *paramName, int errorno) {
  lua_getglobal(L, "modelErr");
  lua_pushnumber(L, ncore);
  lua_pushnumber(L, nlocal);
  lua_pushstring(L, ntype);
  lua_pushstring(L, paramName);
  lua_pushnumber(L, errorno);
  lua_pushstring(L, NEMO_MODEL_FILE_PATH);

  lua_call(L, 6, 0);
}

long getLuaArray(long *arr) {

  lua_pushvalue(L, -1);
  lua_pushnil(L);
  int elnum = 0;
  while (lua_next(L, -2)!=0) {
    lua_pushvalue(L, -2);

    const char *key = lua_tostring(L, -1);
    long value = lua_tointeger(L, -2);
    arr[elnum] = value;
    elnum++;
    lua_pop(L, 2);
  }
  lua_pop(L, 1);

  return elnum - 1;
}

long long lGetParam(int isArray, long *arrayParam) {

  if (isArray) {
    return getLuaArray(arrayParam);
  } else {
    long long value = lua_tointeger(L, -1);
    return value;
  }

  return -1;
}

void clearNeuron(int curCoreID, int curLocalID) {
  lua_getglobal(L, "clearNeuron");
  lua_pushinteger(L, curCoreID);
  lua_pushinteger(L, curLocalID);
  lua_pushstring(L, "TN");
  lua_call(L, 3, 0);
}
//check to see if a neuron exists in the config file
bool neuronExists() {
  lua_getglobal(L, "doesNeuronExist");
  lua_pushnumber(L, curCoreID);
  lua_pushnumber(L, curLocalID);
  lua_pushstring(L, "TN");
  lua_call(L, 3, 1);
  return (lua_toboolean(L, -1)==true);

}

int lookupAndPrimeNeuron(long coreID, long localID, char *nt) {
  curCoreID = coreID;
  curLocalID = localID;
  curType = nt;

  if (neuronExists()) {
    if (DBG_MODEL_MSGS) {
      printf("NeuronExists - %li_%s_%li \n", coreID, nt, localID);
    }
    return 0;
  } else {
    curCoreID = 0;
    curLocalID = 0;
    curType = "";
  }
  return -1;
}

long lGetAndPushParam(char *paramName, int isArray, long *arrayParam) {

  lPushParam(paramName);
  return lGetParam(isArray, arrayParam);

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

char *luT(char *nemoName) {
  lua_getglobal(LT, nemoName);
  char *vname = (char *) lua_tostring(LT, -1);
  lua_pop(LT, 1);
  return vname;

}

void closeLua() {
  if (L) {
    lua_close(L);
    if (g_tw_npe <= MAX_RANKS_FILES) {
      if (g_tw_mynode==0) {
        tw_printf(TW_LOC, "Load complete - freeing config file.\n");
      }
      free(luaConfigFile);
    }
  }
}

void clearStack() {
  lua_settop(L, 0);
}
