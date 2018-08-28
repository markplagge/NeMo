//
// Created by Mark Plagge on 8/8/18.
//

#include "tn_parser.hh"

int char2int(char input) {

  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

void int2bit(int input, bool v[4]) {
  char binary[16][5] =
      {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101",
       "1110", "1111"};

}
/** @defgroup nemo_import NeMoFunct
 * These functions are imported from NeMo. They define various neuron creation functions.
 * Rather than link this JSON lib into NeMo (making a NeMo Library), I re-defined the basic
 * creation functions here.
 */
/**
 * Extracted from NeMo
 * @param core
 * @param coreLocal
 * @return
 */
tw_lpid getGIDFromLocalIDs(id_type core, id_type coreLocal) {

  return (core * CORE_SIZE) + coreLocal;
}

tw_lpid getNeuronGlobal(id_type core, id_type neuronID) {
  id_type coreLocal = AXONS_IN_CORE + SYNAPSES_IN_CORE + neuronID;
  return getGIDFromLocalIDs(core, coreLocal);
}

void TN_set_neuron_dest(int signalDelay, uint64_t gid, tn_neuron_state *n) {
  n->delayVal = signalDelay;
  n->outputGID = gid;

}
void tn_create_neuron(id_type coreID, id_type nID,
                      bool synapticConnectivity[NEURONS_IN_CORE],
                      short G_i[NEURONS_IN_CORE], short sigma[4], short S[4],
                      bool b[4], bool epsilon, short sigma_l, short lambda,
                      bool c, uint32_t alpha, uint32_t beta, short TM, short VR,
                      short sigmaVR, short gamma, bool kappa,
                      tn_neuron_state *n, int signalDelay,
                      uint64_t destGlobalID, int destAxonID) {
  for (int i = 0; i < 4; i++) {
    n->synapticWeight[i] = sigma[i] * S[i];
    n->weightSelection[i] = b[i];
  }
  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    n->synapticConnectivity[i] = synapticConnectivity[i];
    n->axonTypes[i] = G_i[i];
  }

  // set up other parameters
  n->myCoreID = coreID;
  n->myLocalID = nID;
  n->epsilon = epsilon;
  n->sigma_l = sigma_l;
  n->lambda = lambda;
  n->c = c;
  n->posThreshold = alpha;
  n->negThreshold = beta;
  // n->thresholdMaskBits = TM;
  // n->thresholdPRNMask = getBitMask(n->thresholdMaskBits);
  n->sigmaVR = SGN(VR);
  n->encodedResetVoltage = VR;
  n->resetVoltage = VR;  //* sigmaVR;

  n->resetMode = gamma;
  n->kappa = kappa;


  //! @TODO: perhaps calculate if a neuron is self firing or not.
  n->firedLast = false;
  n->heartbeatOut = false;
  // n->isSelfFiring = false;
  // n->receivedSynapseMsgs = 0;

  TN_set_neuron_dest(signalDelay, destGlobalID, n);

  // synaptic neuron setup:
  n->largestRandomValue = n->thresholdPRNMask;
  if (n->largestRandomValue > 256) {
    tw_error(TW_LOC, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ",
             n->myCoreID, n->myLocalID);
  }
  // just using this rather than bit shadowing.

  n->dendriteLocal = destAxonID;
  n->outputGID = destGlobalID;

  // Check to see if we are a self-firing neuron. If so, we need to send
  // heartbeats every big tick.
  n->isSelfFiring =
      false;  //!@TODO: Add logic to support self-firing (spontanious) neurons
}

void tn_create_neuron_encoded_rv(
    id_type coreID, id_type nID, bool synapticConnectivity[NEURONS_IN_CORE],
    short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
    bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
    uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa,
    tn_neuron_state *n, int signalDelay, uint64_t destGlobalID,
    int destAxonID) {
  tn_create_neuron(coreID, nID, synapticConnectivity, G_i, sigma, S, b, epsilon,
                   sigma_l, lambda, c, alpha, beta, TM, VR, sigmaVR, gamma,
                   kappa, n, signalDelay, destGlobalID, destAxonID);
  n->sigmaVR = sigmaVR;
  n->encodedResetVoltage = VR;
  n->resetVoltage = (int) (n->sigmaVR * (pow(2, n->encodedResetVoltage) - 1));
}

void tn_create_neuron_encoded_rv_non_global(
    int coreID, int nID, bool synapticConnectivity[NEURONS_IN_CORE],
    short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
    bool epsilon, int sigma_l, int lambda, bool c, int alpha,
    int beta, int TM, int VR, int sigmaVR, int gamma, bool kappa,
    tn_neuron_state *n, int signalDelay, int destCoreID,
    int destAxonID) {
  uint64_t dest_global = getNeuronGlobal(destCoreID, destAxonID);
  tn_create_neuron_encoded_rv(coreID,
                              nID,
                              synapticConnectivity,
                              G_i,
                              sigma,
                              S,
                              b,
                              epsilon,
                              sigma_l,
                              lambda,
                              c,
                              alpha,
                              beta,
                              TM,
                              VR,
                              sigmaVR,
                              gamma,
                              kappa,
                              n,
                              signalDelay,
                              dest_global,
                              destAxonID);
}

/**
 * hex2bool_insert
 * Converts a hex char into a boolean array, representing the synapse connectivity matrix. Done
 * using hard-coded values to ensure correct ordering across multiple platforms.
 * @param hex
 * @return
 */
vector<bool> hex2bool_insert(char hex) {

  bool boolv[16][4] = {{false, false, false, false}, {false, false, false, true}, {false, false, true, false},
                       {false, false, true, true}, {false, true, false, false}, {false, true, false, true},
                       {false, true, true, false}, {false, true, true, true}, {true, false, false, false},
                       {true, false, false, true}, {true, false, true, false}, {true, false, true, true},
                       {true, true, false, false}, {true, true, false, true}, {true, true, true, false},
                       {true, true, true, true}};
  int cv = char2int(hex);
  vector<bool> return_val(boolv[cv], boolv[cv] + 4);
  //bool_vec.insert(bool_vec.end(),boolv[cv],boolv[cv]+4);

  return return_val;
  /*
case '0' :
  return_val = {false,false,false,false};
  break;
case '1' :
  return_val = {false,false,false,true};
  break;
case '2' :
  return_val = {false,false,true,false};
  break;
case '3' :
  return_val = {false,false,true,true};
  break;
case '4' :
  return_val = {false,true,false,false};
  break;
case '5' :
  return_val = {false,true,false,true};
  break;
case '6' :
  return_val = {false,true,true,false};
  break;
case '7' :
  return_val = {false,true,true,true};
  break;
case '8' :
  return_val = {true,false,false,false};
  break;
case '9' :
  return_val = {true,false,false,true};
  break;
case 'A' :
  return_val = {true,false,true,false};
  break;
case 'B' :
  return_val = {true,false,true,true};
  break;
case 'C' :
  return_val = {true,true,false,false};
  break;
case 'D' :
  return_val = {true,true,false,true};
  break;
case 'E' :
  return_val = {true,true,true,false};
  break;
case 'F' :
  return_val = {true,true,true,true};
  break;

  } */

}
/**
 * fullhex2bool -
 * given a synaptic connectivity line (from the TN JSON file), returns a vector of bools representing the
 * connectivity matrix.
 * @param hexstr
 * @return
 */
vector<bool> fullhex2bool(string hexstr) {
  vector<bool> bool_vec;
  for (auto it: hexstr) {
    if (it != ' ') {
      vector<bool> single = hex2bool_insert(it);
      bool_vec.insert(bool_vec.end(), single.begin(), single.end());
    }
  }
  return bool_vec;

}




//def createNeuronTemplateFromEntry(line, nSynapses=256, weights=4):
//"""
//Function that creates a new neuron from a dictionary. The dictionary is parsed from a line in the JSON file.
//:param line: The JSON line in a dictionary format
//:param nSynapses: number of synapses in the core
//:param weights: number of weights in the core
//:return: A new neuron shiny and chrome
//"""
//neuron = TN_Wrapper(nSynapses, weights)
//sigmas = []
//s = []
//b = []
//for i in range(0, weights):
//sigmas.append(line[f'sigma{i}'])
//s.append(line[f's{i}'])
//bs = line[f'b{i}']
//bs = 0 if bs == False else 1
//
//b.append(bs)
//
//neuron.S = s
//neuron.sigmaG = sigmas
//neuron.epsilon = line['epsilon']
//neuron.sigma_lmbda = line['sigma_lambda']
//neuron.lmbda = line['lambda']
//neuron.c = line['c_lambda']
//neuron.epsilon = line['epsilon']
//neuron.alpha = line['alpha']
//neuron.beta = line['beta']
//neuron.TM = line['TM']
//neuron.gamma = line['gamma']
//neuron.kappa = line['kappa']
//neuron.sigmaVR = line['sigma_VR']
//neuron.VR = line['VR']
//neuron.membrane_potential = line["V"]
//
//return neuron


const string &TN_Wrapper::getName() const {
  return name;
}
void TN_Wrapper::setName(const string &name) {
  TN_Wrapper::name = name;
}

void NeuronTypeLib::gen_type_map(Value &n_types) {
  for (auto &v : n_types.GetArray()) {
    TN_Wrapper n(v);
    //this->neuron_types.insert(std::pair<string,TN_Wrapper>(n.getName(),n));
    this->neuron_types.insert({n.getName(), n});
  }
}
TN_Wrapper NeuronTypeLib::get_neuron(string name) {
  return this->neuron_types.at(name);
}

void TN_Crossbar_Type::add_synapse_rows(vector<string> types, vector<string> synapses) {
  for (int i = 0; i < synapses.size(); i++) {
    vector<bool> synrow = fullhex2bool(synapses[i]);
    rows.push_back(synrow);
    types.push_back(types[i]);
  }
}

vector<bool> TN_Crossbar_Type::get_connectivity(int neuron) {
  // given a neuron position, returns the column of neurons.
  vector<bool> connectivity_row;
  for (int i = 0; i < rows.size(); i++) {
    connectivity_row.push_back(rows[i][neuron]);
  }
  return connectivity_row;
}

void TN_Neuron_Type::set_name(std::string name) {
  int id = 0;
  this->name.append(name);
  name.erase(name.begin());
  this->name_id = std::stoul(name, nullptr, 16);

}
void TN_Neuron_Type::init_neuron_from_json_doc(rapidjson::Document json_doc) {
  // for each variable a neuron needs, parse it:

}

tn_neuron_state *TN_Neuron_Type::new_neuron_state(vector<int> input_axon_connectivity,
                                                  vector<short> input_axon_types,
                                                  int output_core,
                                                  int output_neuron,
                                                  int source_core,
                                                  int source_local,
                                                  int dest_delay) {

  tn_neuron_state *n = (tn_neuron_state *) calloc(1, sizeof(tn_neuron_state));
  //set up the variables // arrays
  map<string, int> v = var_types;
  //vector<short> sigma ({(short)v["sigma0"],(short)v["sigma1"],(short)v["sigma2"],(short)v["sigma3"]});
  short sigma[4];
  sigma[0] = (short) v["sigma0"];
  sigma[1] = (short) v["sigma1"];
  sigma[2] = (short) v["sigma2"];
  sigma[3] = (short) v["sigma3"];
  short S[4];
  S[0] = (short) v["s0"];
  S[1] = (short) v["s1"];
  S[2] = (short) v["s2"];
  S[3] = (short) v["s3"];
  bool b[4];
  b[0] = (bool) v["b0"],
  b[1] = (bool) v["b1"],
  b[2] = (bool) v["b2"];
  b[3] = (bool) v["b3"];

  n->synapticWeight[0] = var_types["sigma0"];
  n->synapticWeight[1] = var_types["sigma1"];
  n->synapticWeight[2] = var_types["sigma2"];
  n->synapticWeight[3] = var_types["sigma3"];
  n->weightSelection[0] = var_types["b0"];
  n->weightSelection[1] = var_types["b1"];
  n->weightSelection[2] = var_types["b2"];
  n->weightSelection[3] = var_types["b3"];

  n->lambda = var_types["lambda"];
  n->sigma_l = var_types["sigma_lambda"];
  n->c = var_types["c_lambda"];
  n->epsilon = var_types["epsilon"];
  n->posThreshold = var_types["alpha"];
  n->negThreshold = var_types["beta"];
  n->sigmaVR = var_types["sigma_VR"];
  n->resetMode = var_types["gamma"];
  bool *synapticConnectivity = (bool *) input_axon_connectivity.data();
  short *input_axon_tps = input_axon_types.data();

  tn_create_neuron_encoded_rv_non_global(source_core,
                                         source_local,
                                         synapticConnectivity,
                                         input_axon_tps,
                                         sigma,
                                         S,
                                         b,
                                         (bool) v["epsilon"],
                                         v["sigma_lambda"],
                                         v["lambda"],
                                         (bool) v["c_lambda"],
                                         v["alpha"],
                                         v["beta"],
                                         v["TM"],
                                         v["VR"],
                                         v["sigma_VR"],
                                         v["gamma"],
                                         (bool) v["kappa"],
                                         n,
                                         dest_delay,
                                         output_core,
                                         output_neuron);
  n->membranePotential = v["V"];
//    tn_create_neuron_encoded_rv_non_global(source_core, <#id_type nID#>, <#bool *synapticConnectivity#>, <#short *G_i#>, <#short *sigma#>, <#short *S#>, <#bool *b#>, <#bool epsilon#>, <#short sigma_l#>, <#short lambda#>, <#bool c#>, <#uint32_t alpha#>, <#uint32_t beta#>, <#short TM#>, <#short VR#>, <#short sigmaVR#>, <#short gamma#>, <#bool kappa#>, <#tn_neuron_state *n#>, <#int signalDelay#>, <#uint64_t destCoreID#>, <#int destAxonID#>)
//
//
//    tn_create_neuron_encoded_rv_non_global((id_type) source_core, (id_type) source_local, input_axon_connectivity.data(), G_i, sigma, S, b, v["epsilon"], v["sigma_lambda"], v["lambda"], v["c"], v["alpha"], v["beta"], v["TM"], v["VR"], v["sigma_VR"], v["gamma"], v["kappa"], n, dest_delay, output_core, output_neuron);
  return n;

}

string TN_Neuron_Type::get_name() {
  return name;
}
int TN_Neuron_Type::get_name_id() {
  return name_id;
}
void TN_Neuron_Type::init_neuron_from_json_arr(rapidjson::GenericValue<rapidjson::UTF8<char>,
                                                                       rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> > &json_neuron) {
  //cout << "Neuron Name: " << json_array["name"].GetString();


  for (auto &it : json_neuron.GetObject()) {
    int type = var_types[it.name.GetString()];
    string key = it.name.GetString();
    cout << it.name.GetString() << '\n';
    //store the vaules in the map - this is the easier way to do this:
    int val = -999;
    if (type == 0) { // name
      set_name(it.value.GetString());
    } else if (type == 1) { // class
      n_class = it.value.GetString();
    } else { // everything else!
      //Is this a string?


      if (it.value.IsBool()) {
//                string f = "false";
//                string valn = it.value.GetString();
//                val = 0;
//                if (valn == f) {
//                    val = 1;
//                }
        bool v2 = it.value.GetBool();
        val = v2;
      } else {
        val = it.value.GetInt();

      }

    }
    var_types[key] = val;
//        switch (type) {
//            case 0: //name
//                //var_types[key] = it.value.GetString();
//                set_name(it.value.GetString());
//                break;
//            case 1: //
//                n_class = it.value.GetString();
//                break;
//            case 2:
//            case 3:
//            case 4:
//            case 5:
//            case 6:
//            case 7:
//            case 8:
//                break;
//
//            default:
//                break;
//        }
    // We don't need to do this - however, it is handy for debugging purposes
  }
}

/**
 * Initializes a core from a json iterator.
 * @param obj
 */
//void TN_Core::init(rapidjson::GenericObject obj){

//}

/**
 * Returns a map<string, TN_NeuronType> which represents a neuron type library.
 * Note that the key in the map points to the JSON neuron number ID
 * (which must be extracted from the string, and is retrieved from TN_Neuron_Type::get_name_id
 * @param json_doc
 * @return
 */
map<int, TN_Neuron_Type> generate_neurons_from_json(rapidjson::Document &json_doc) {
  map<int, TN_Neuron_Type> neuron_lib;
  //create a map, a new TN Neuron Type, and append it to the map for
  //each neuron prototype in the TN Json File.
  auto n_type_arr = json_doc["neuronTypes"].GetArray();
  for (auto &it : n_type_arr) {
    TN_Neuron_Type n;
    n.init_neuron_from_json_arr(it);
    neuron_lib[n.get_name_id()] = n;
  }
  return neuron_lib;
}
/*"core":{
		"metadata":{
			"coreletClass":"th_corelet_layer_cores",
			"coreletId":2,
			"coreNumber":4,
			"parentCoreletId":[3,1,0],
			"layerNumber":2,
			"layerType":"conv"
		},
		"id":4,
		"timeScaleExponent":0,
		"rngSeed":4294967295,

"neurons":{

"dendrites":["0:251","252:255"],
"types":[25,28,23,22,24,24,12,22,27,33,23,27,39,15,23,23,44,18,31,22,30,16,31,6,36,23,18,33,14,31,26,18,24,31,32,31,17,29,23,19,36,12,33,17,40,28,30,23,27,21,23,14,12,29,23,24,32,17,31,13,22,18,27,22,25,25,10,20,28,"18:2:22",30,21,14,24,21,14,26,30,15,32,22,25,20,30,22,13,31,24,18,21,22,26,21,28,24,33,19,23,34,33,23,33,25,19,22,22,30,22,34,23,28,23,25,25,17,6,12,28,23,22,16,8,37,"22:3:28",23,22,24,24,12,22,27,33,23,27,39,15,23,23,44,18,31,22,30,16,31,6,36,23,18,33,14,31,26,18,24,31,32,31,17,29,23,19,36,12,33,17,40,28,30,23,27,21,23,14,12,29,23,24,32,17,31,13,22,18,27,22,25,25,10,20,28,"18:2:22",30,21,14,24,21,14,26,30,15,32,22,25,20,30,22,13,31,24,18,21,22,26,21,28,24,33,19,23,34,33,23,33,25,19,22,22,30,22,34,23,28,23,25,25,17,6,12,28,23,22,16,8,37,22,"0x4"],
"destCores":[878,878,"1134x4",878,"1134x5",878,1134,878,1134,"878x4",1134,1134,878,1134,"878x3",1134,1134,878,1134,878,"1134x3",878,1134,1134,878,878,1134,1134,878,1134,878,1134,878,"1134x5","878x3",1134,878,1134,878,1134,"878x3",1134,1134,878,"1134x4",878,1134,1134,878,1134,878,878,"1134x3",878,1134,"878x4",1134,878,1134,878,1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,1134,"878x4","1134x3",878,1134,878,878,"1134x4",878,"1134x5",878,1134,878,1134,"878x4",1134,1134,878,1134,"878x3",1134,1134,878,1134,878,"1134x3",878,1134,1134,878,878,1134,1134,878,1134,878,1134,878,"1134x5","878x3",1134,878,1134,878,1134,"878x3",1134,1134,878,"1134x4",878,1134,1134,878,1134,878,878,"1134x3",878,1134,"878x4",1134,878,1134,878,1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,1134,"878x4","1134x3",878,1134,"-1x4"],
"destAxons":[66,222,210,150,238,96,12,184,222,98,136,122,208,94,86,112,8,156,18,42,174,108,172,232,10,234,204,138,156,96,12,240,84,114,132,154,102,92,78,54,16,240,182,62,108,128,136,18,110,196,236,42,64,72,138,180,134,166,130,218,184,102,132,72,2,190,178,60,44,212,158,134,130,202,50,26,22,32,144,224,58,52,250,2,178,76,152,170,30,80,68,150,90,106,126,244,34,114,208,74,176,36,250,30,238,46,242,0,140,168,188,36,212,142,160,228,204,88,152,70,62,190,58,46,68,186,67,223,211,151,239,97,13,185,223,99,137,123,209,95,87,113,9,157,19,43,175,109,173,233,11,235,205,139,157,97,13,241,85,115,133,155,103,93,79,55,17,241,183,63,109,129,137,19,111,197,237,43,65,73,139,181,135,167,131,219,185,103,133,73,3,191,179,61,45,213,159,135,131,203,51,27,23,33,145,225,59,53,251,3,179,77,153,171,31,81,69,151,91,107,127,245,35,115,209,75,177,37,251,31,239,47,243,1,141,169,189,37,213,143,161,229,205,89,153,71,63,191,59,47,69,187,"0x4"],
"destDelays":["1x252","1x4"]
},

"crossbar":{
"name":"coreProt0000001"
}
},*/


typedef struct metadata{
  string coreletClass;
  int coreletID;
  int coreNumber;
  vector<int> parentCoreletID;
  int layerNumber;
  string layerType;
}core_metadata;



/**
 * converts string values to an integer array - based on TN spec.
 * @param tn_value
 */
 vector<int> convert_tn_arr(const string &tn_value){
  std::size_t found_c = tn_value.find(':');
  std::size_t found_x = tn_value.find('x');
  std::string segment;
  std::vector<std::string> seglist;
  std:stringstream tn_value_str(tn_value);
  vector<int> result;

  if(found_c!=std::string::npos) {
    int num_cols = 0;
    //Found a ":" - so we iterate through the values
    while (std::getline(tn_value_str, segment, ':')) {
      num_cols++;
      seglist.push_back(segment);
    }
    if (num_cols == 2) {
      //iterate from pos 0 to pos 1.
      int start = stoi(seglist[0], NULL, 10);
      int end = stoi(seglist[1], NULL, 10);
      int second_end = end + start;
      while (start <= second_end){
        result.push_back(start);
        start ++;
      }
//      for (int i = start; i < end; i++) {
//        result.push_back(i);
//      }
    } else if (num_cols == 3) {
      //two colons means:
      //start:increment:end
      std::string::size_type pos;
      int start = stoi(tn_value,&pos,10);
      pos ++;
      string sub1 = tn_value.substr(pos);
      int increment = stoi(sub1,&pos,10);
      pos ++;
      string sub2 = sub1.substr(pos);
      int end = stoi(sub2, &pos,10);
      while (start <= end){
        result.push_back(start);
        start += increment;
      }
//      for (int i = start; i < end; i ++){
//        result.push_back(start);
//        start += increment;
//      }


    } else {
      cout << "Found " << num_cols << " columns - not a valid number of columns\n";
    }

  }else if(found_x!=std::string::npos){
    std::string::size_type pos;
    int value = stoi(tn_value,&pos,10);
    int num_itr= stoi(tn_value.substr(pos+1),NULL,10);
    for(int i = 0; i < num_itr; i ++){
      result.push_back(value);
    }

  }else{
    cout <<"Error - found something unexpecteD: " << tn_value << "\n";
  }

  return result;

 }

 int convert_and_add_value(int *&vals,string val){
   vector<int> valv = convert_tn_arr(val);
   for(auto it : valv){
     *vals = it;
     vals ++;
   }
 }
 int convert_and_add_value(int *&vals,  int val){
   *vals = val;
   vals ++;
 }
 void convert_and_add_value(int *&vals, const Value & val){
   if (val.IsString()){
     convert_and_add_value(vals,val.GetString());
   }else if(val.IsInt()){
     convert_and_add_value(vals,val.GetInt());
   }else{
     cout << "Invalid conversion value! \n ";

   }
 }



// int convert_and_add_value(int *vals, int arr_pos,rapidjson::GenericObject obj){
//
// }
/**
 * assign (v, name) - macro to reduce code size in the TN_Core::init_core_from_itr() function.
 * Simply extracts the value called name form v, and assigns it to the variabled named name.
 */
#define assign(v, name) (name) = (v)[#name]

/**
* sets up this TN_Core object from the iterator - json doc object.
* @param itr
*/
void TN_Core::init_core_from_itr(Value::ConstMemberIterator itr) {
  //itr->value.GetObject()
  auto main_obj = itr->value.GetObject();
  auto meta_obj = main_obj["metadata"].GetObject();
  auto neuron_obj = main_obj["neurons"].GetObject();
//  coreletClass = ob["coreletClass"].GetString();
//  coreletID = ob["coreletId"].GetInt();
//  coreNumber = ob["coreNumber"].GetInt();


  //base core data:
  //"id":4,
  //		"timeScaleExponent":0,
  //		"rngSeed":4294967295,
  assign(main_obj, timeScaleExponent).GetInt();
  assign(main_obj, rngSeed).GetUint();
  assign(main_obj, id).GetUint();


  // Metadata ----
  //		"metadata":{
  //			"coreletClass":"th_corelet_layer_cores",
  //			"coreletId":2,
  //			"coreNumber":4,
  //			"parentCoreletId":[3,1,0],
  //			"layerNumber":2,
  //			"layerType":"conv"

  assign(meta_obj,coreletClass).GetString();
  assign(meta_obj,coreletId).GetInt();
  assign(meta_obj,coreNumber).GetInt();
  assign(meta_obj,layerNumber).GetInt();
  assign(meta_obj,layerType).GetString();



  crossbar_name = main_obj["crossbar"]["name"].GetString();

  // Parse Arrays:
  for (auto &el : main_obj["metadata"]["parentCoreletId"].GetArray()){
    parentCorletId.push_back(el.GetInt());
  }
  //Parse the dendrites, types, destCores, destAxons, destDelays arrays:
  //dendrites:
  //Debug Code:
  vector<int> testDendrite;

  int arr_pos = 0;
#ifdef DEBUG
  for (auto &el : neuron_obj["dendrites"].GetArray()){
    if (el.IsString()){
      vector<int> vals = convert_tn_arr(el.GetString());
      for (auto it : vals){
        dendrites[arr_pos] = it;
        arr_pos ++;

        testDendrite.push_back(it);

      }
    }else{
      dendrites[arr_pos] = el.GetInt();

      testDendrite.push_back(el.GetInt());

      arr_pos ++;
    }
  }
#endif

  //Test code:
  arr_pos = 0;
  int *d_ptr = dendrites;
  for(auto &el : neuron_obj["dendrites"].GetArray()){
    convert_and_add_value(d_ptr,el);
  }
#ifdef DEBUG
  int test_v = 0;
  for(auto x: testDendrite){
    if (dendrites[test_v] != x){
      cout << "Found error in dendrite array.\n";
    }
    test_v ++;
  }
#endif
  // added function wrapper for the next ones:
  arr_pos = 0;
  int *t_ptr = types;
  for (auto &el : neuron_obj["types"].GetArray()){
    convert_and_add_value(t_ptr,el);
  }
  int *dc_ptr = destCores;
  for (auto &el : neuron_obj["destCores"].GetArray()){
    convert_and_add_value(dc_ptr,el);
  }
  int *da_ptr = destAxons;
  for (auto &el : neuron_obj["destAxons"].GetArray()) {
    convert_and_add_value(da_ptr,el);
  }
  int *dd_ptr = destDelays;
  for (auto &el : neuron_obj["destDelays"].GetArray()) {
    convert_and_add_value(dd_ptr,el);
  }



//  assign("parentCoreletId").GetArray();




}

/**
 * Given a json_doc (rapidjson), iterate through the cores and parse them.
 * Each core's object is sent to the TN_Core::generate_core_from_itr() function.
 * @param json_doc
 * @return
 */
map<int, TN_Core> generate_cores_from_json(rapidjson::Document &document) {
  map<int, TN_Core> core_lib;

  for (Value::ConstMemberIterator itr = document.FindMember("core"); itr != document.MemberEnd(); ++itr) {
    TN_Core core;
    core.init_core_from_itr(itr);
    core_lib[itr->value.GetObject()["id"].GetInt()] = core;
  }

  return core_lib;
}

//TN_Neuron_Type TN_Main::generate_neuron_from_id(int coreID, int neuronID) {
  //first we need the core:
  TN_Core core = TN_Cores[to_string(coreID)];
  //next, each core has
  TN_Crossbar_Type crossbar = TN_Crossbar_Type_library[]
}




/**
 * create_tn_data: preps the neuron model from the json file. Given a JSON file (TN FORMAT),
 * this will generate the crossbar array templates, the neuron templates, and then generate the
 * core templates. Once done, the TN_Main object is returned, which can generate NeMo TN LPs.
 * @param filename The TN Filename
 * @return a new TN_Main .
 */
TN_Main create_tn_data(string filename) {

//  fstream json_file(filename);
//  //    char x[40];
//  //    json_file.read(x,30);
//  //    cout << x;
//
//  std::string json_str((std::istreambuf_iterator<char>(json_file)),
//                       std::istreambuf_iterator<char>());
  string json_str = load_file_into_memory(filename);

  Document json_doc;
  json_doc.Parse<kParseCommentsFlag>(json_str);

  //First create the main core object
  TN_Main model;
  //populate the model data.
  model.core_count = json_doc["model"]["coreCount"].GetInt();
  model.crossbar_size = json_doc["model"]["crossbarSize"].GetInt();
  model.crossbar_class = json_doc["model"]["crossbarclass"].GetString();

  //now create the crossbar prototypes.
  map<string, TN_Crossbar_Type> crossbar_library;
  for (auto &v : json_doc["crossbarTypes"].GetArray()) {
    string crossbarName = v["name"].GetString();
    vector<string> types;
    vector<string> synapses;
    for (auto &r : v["crossbar"]["rows"].GetArray()) {
      types.push_back(r["type"].GetString());
      string row = r["synapses"].GetString();
      synapses.push_back(row);

    }
    TN_Crossbar_Type tp;
    tp.add_synapse_rows(types, synapses);
    tp.name = crossbarName;
    crossbar_library[crossbarName] = tp;
  }
  //add the crossbar prototypes to the main TN model
  model.TN_Crossbar_Type_library = crossbar_library;
  //Now parse the neuron prototypes  -
  map<int, TN_Neuron_Type> neuron_types = generate_neurons_from_json(json_doc);
  model.TN_Neuron_Library = neuron_types;

  auto n_type_arr = json_doc["neuronTypes"].GetArray();
//  for (auto &it : n_type_arr) {
//    cout << "NTI" << it["name"].GetString() << "\n";
//  }

  map<int, TN_Core> core_lib;
  core_lib = generate_cores_from_json(json_doc);

  //return the main model.

  return model;
}

