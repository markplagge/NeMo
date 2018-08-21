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

vector<bool> fullhex2bool(string hexstr) {
  vector<bool> bool_vec;
  for (auto it: hexstr) {
    vector<bool> single = hex2bool_insert(it);
    bool_vec.insert(bool_vec.end(), single.begin(), single.end());
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

void read_TN(string filename) {

}

map<string, TN_Wrapper> neuron_types_to_map(Value &n_types) {
  map<string, TN_Wrapper> neuron_types;
  //iterate through json value array and create templates!
//  for (auto& v : a.GetArray()){
//
//  }

  return neuron_types;
}
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

string TNParser::create_tn_string(string prototype_name) {

}
string TNParser::create_tn_string() {

}
