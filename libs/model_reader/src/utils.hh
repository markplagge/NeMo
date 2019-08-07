//
// Created by Mark Plagge on 8/23/18.
//





#ifndef SUPERNEMO_UTILS_HH
#define SUPERNEMO_UTILS_HH
#include <cstdlib>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>
std::string load_file_into_memory(std::string filename);
bool replace(std::string& str, const std::string& from, const std::string& to);
//std::string regex_full_core;

#endif //SUPERNEMO_UTILS_HH
