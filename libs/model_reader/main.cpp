//
// Created by Mark Plagge on 8/8/18.
//


#include <cstdio>
#include "include/tn_parser.hh"
#include "extern/rang.hh"
#include "extern/CLI11.hh"
#include "extern/argh.hh"
#include "tests/test_data.hh"
#define RAPIDJSON_PARSE_DEFAULT_FLAGS KParseCommentFlags

void signal_handler(int s){
  std::cout << std::endl << rang::style::reset << rang::fg::red << rang::style ::bold;
  std::cout << "Control-C detected, exiting..." << rang::style::reset << std::endl;
  std::exit(1); // will call the correct exit func, no unwinding of the stack though
}

#ifdef DEBUG
#define RRED(x) rang::style::reset << rang::fg::red << (x) << rang::style::reset
#define RBLU(x) rang::style::reset << rang::fg::blue << (x) << rang::style::reset
#define pl cout << "At line " << RBLU(__LINE__ ) << "\n"
#define pause cout << RBLU("Press enter... \n"); cin >> temp;

void test_bgr(string filename){
  cout << "New code. \n";
  int temp;
  Document c_str_test;
//  std::vector<char> data(test_cstr_big, test_cstr_big+ size);
  cout << "Starting tests of secondary JSON library.\n";
//  char short_test[] = "{\n"
//                      "  /* General model parameters */\n"
//                      "  \"model\":{\n"
//                      "    \"coreCount\":4042,\n"
//                      "    \"neuronclass\":\"NeuronGeneral\",\n"
//                      "    \"crossbarSize\":256,\n"
//                      "    \"crossbarclass\":\"CrossbarBinary\",\n"
//                      "    \"buildKey64\":\"20180312T181135_CPEmakeModel_37D8A05D623E7F6AC67EECBF929EDFBBE09\",\n"
//                      "    \"networking\":\"INTRA\",\n"
//                      "    \"useCrossbarPrototypes\":true\n"
//                      "  },\n"
//                      "  /* Crossbar type library */\n"
//                      "  \"crossbarTypes\":[\n"
//                      "    {\n"
//                      "      \"name\":\"coreProt0002603\",\n"
//                      "      \"crossbar\":{\n"
//                      "        /* Row parameters */\n"
//                      "        \"rows\":[\n"
//                      "          /* Parameters */\n"
//                      "          {\"type\":\"S0\",\"synapses\":\"51 40 C0 90 90 4B 83 00 12 80 02 01 18 38 A0 40 51 40 C0 90 90 4B 83 00 12 80 02 01 18 38 A0 40\"},\n"
//                      "          {\"type\":\"S1\",\"synapses\":\"A0 00 11 61 04 84 00 B8 08 24 19 70 41 00 0C 03 A0 00 11 61 04 84 00 B8 08 24 19 70 41 00 0C 03\"}]}\n"
//                      "}]}";
char short_test[] = "{\n"
                    "  \"model\":{\n"
                    "    \"coreCount\":4042,\n"
                    "    \"neuronclass\":\"NeuronGeneral\",\n"
                    "    \"crossbarSize\":256,\n"
                    "    \"crossbarclass\":\"CrossbarBinary\",\n"
                    "    \"buildKey64\":\"20180312T181135_CPEmakeModel_37D8A05D623E7F6AC67EECBF929EDFBBE09\",\n"
                    "    \"networking\":\"INTRA\",\n"
                    "    \"useCrossbarPrototypes\":true\n"
                    "  },\n"
                    "  \"crossbarTypes\":[\n"
                    "    {\n"
                    "      \"name\":\"coreProt0002603\",\n"
                    "      \"crossbar\":{\n"
                    "            \"rows\":[\n"
                    "              {\"type\":\"S0\",\"synapses\":\"51 40 C0 90 90 4B 83 00 12 80 02 01 18 38 A0 40 51 40 C0 90 90 4B 83 00 12 80 02 01 18 38 A0 40\"},\n"
                    "          {\"type\":\"S1\",\"synapses\":\"A0 00 11 61 04 84 00 B8 08 24 19 70 41 00 0C 03 A0 00 11 61 04 84 00 B8 08 24 19 70 41 00 0C 03\"}]}\n"
                    "}]}";
  char buffer[sizeof(short_test)];
  memcpy(buffer, short_test, sizeof(short_test));
  cout << " Buffer is: \n" << buffer << "\n";
  Document jx;
  cout << "Endian set to (1 is big): " << RAPIDJSON_ENDIAN << "\n" << "is 64 bit? " << RAPIDJSON_64BIT << "\n";
  //jx.Parse<kParseCommentsFlag>(buffer);
  string cen = kParseCommentsFlag & kParseDefaultFlags ? " Enabled ":" Disabled";
  cout <<"Default flags set - comments: " << cen <<"\n";
  jx.Parse(short_test);
  cout << "JX Result: " << jx.GetParseError() << " " << rapidjson::GetParseError_En(jx.GetParseError()) << " \n ";
  cout << "JX done: " << RRED(jx["model"]["networking"].GetString()) << "\n";
  cout << " ---- \n ";

  Document d;
  pl;
  d.Parse<kParseCommentsFlag>(test_cstr_big);
  pl;
  TN_Main model;
  pl;
  cout << "Result : " << d.GetParseError() << "CODE: " << rapidjson::GetParseError_En(d.GetParseError()) << " \n";

  cout << "Member 'model': " << RRED(d.HasMember("model")) << "\n";
  pl;
  cout << "Member 'model':coreCount: " << RRED(d["model"].HasMember("coreCount")) << "\n";
  cout << d["model"]["coreCount"].GetInt();
  pl;
  model.core_count = d["model"]["coreCount"].GetInt();
  pl;
  cout << "Core Count: " << model.core_count << "\n";
  pl;

}

int test_file_reads(){
    const char* boot_camp_fn = "./tests/BootCampExample11.json";
    const char* th_test_fn = "./tests/tn_test_cf100.json";
    Document boot_camp;
    FILE* fp = fopen(boot_camp_fn,"r");
    char readBuffer[65535];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    boot_camp.ParseStream<kParseCommentsFlag>(is);
    cout << "Result was " << GetParseError_En( boot_camp.GetParseError()) << " \n";
    
    cout << boot_camp["model"]["coreCount"].GetInt() << " <- CC \n";
    cout << RRED("Finished first test.\n");
    TN_Main model = create_tn_data(boot_camp_fn);
    TN_Output out("./BootCampEx_parsed.json",model,TN_OUT_JSON);
    out.write_data();

    fclose(fp);

    Document th_test;
    FILE* fp2 = fopen(th_test_fn, "r");
    char readBuffer2[65535];
    FileReadStream is2(fp, readBuffer2, sizeof(readBuffer));
    th_test.ParseStream<kParseCommentsFlag>(is2);
    cout << "\n\n ------------------- \n  CF100 Test Result: " << GetParseError_En(th_test.GetParseError()) << "\n";
    cout << th_test["model"]["coreCount"].GetInt() << " <- CC \n";



}
#endif
int main(int argc, char *argv[]) {

  CLI::App app{"NeMo JSON Parser"};

  string filename;
  string save_file;
  bool do_json;
  bool do_bin;
  bool do_nfg;
  bool do_bin_file_per_core;
  bool do_p9_test;
#ifdef DEBUG
  app.add_flag("-p", do_p9_test, "do power9 test")->mandatory(false);
#endif
  app.add_flag("-j", do_json, "Save JSON file - for debug/test.");
  app.add_flag("-b", do_bin, "Save binary file. Please ensure endianness is correct!");
  app.add_flag("-n", do_nfg, "Save NeMo LUA-based config file. Uses the soon-to-be depricated NeMo lua config syntax.");
  app.add_option("-i,--input_file", filename, "Input JSON file")->required()->check(CLI::ExistingFile);
  app.add_option("-o,--output_file", save_file,"Output filename template. This program will automatically add proper "
                                               "extension(s) to the file");
  app.add_flag("-s,--split", do_bin_file_per_core,"When saving binary file, save one file per core rather than"
                                                  "one large file.");

  CLI11_PARSE(app,argc,argv);

#ifdef USE_OMP
  printf("Using OpenMP for loops.\n");
#endif

#ifdef DEBUG
    const char *endian_mode = RAPIDJSON_ENDIAN == RAPIDJSON_LITTLEENDIAN ? "Little Endian" : "Big Endian";
    const char *std_str = RAPIDJSON_HAS_STDSTRING ? "Has STDSTRING" : "No STDSTRING ";


    cout << rang::style::reset << rang::fg::green << rang::style::underline << "Configured RapidJSON Settings: \n" << rang::style::reset;
    cout << rang::fg::blue<< endian_mode << "\n" << rang::style::underline<< "Cx11: " <<rang::style::reset <<
         rang::fg::blue << "RangeFor: " <<RAPIDJSON_HAS_CXX11_RANGE_FOR << "\tNoExcept: "
         << RAPIDJSON_HAS_CXX11_NOEXCEPT << "\trvalue_refs: " << RAPIDJSON_HAS_CXX11_RVALUE_REFS << "\n64 Bit: " << RAPIDJSON_64BIT
         << "\n48 Bit Intel Pack: " << RAPIDJSON_48BITPOINTER_OPTIMIZATION <<"\n" << std_str
         <<"\n" <<  rang::style::reset;
    cout <<rang::style::reset << rang::fg::blue <<"Running debug / testing tests \n" << rang::style::reset;
#  ifdef __BYTE_ORDER__
    cout << "Byte oder defined as " << __BYTE_ORDER__ << "\n";
# elif defined(_LITTLE_ENDIAN)
    cout << "LE Defined as " << _LITLE_ENDIAN <<"\n";
#endif

    if(do_p9_test) {
        cout << "BGRS Testing start \n";
        test_bgr(filename);
    }
test_file_reads();
#endif
  if(!do_json && !do_bin && !do_nfg) {
    cout << std::endl << rang::style::reset << rang::fg::red << rang::style::bold;
    cout << "You must specify at least one of [-j,-b,-n]" << rang::style::reset <<std::endl;
    cout << rang::fg::blue << "Exiting...." << rang::style::reset << endl;
    exit(2);
  }

  unsigned char o = 0;
  if(do_json){
    o = o | TN_OUT_JSON;
  }
  if (do_bin){
    o = o | TN_OUT_BIN;
    if(do_bin_file_per_core){
      o = o | TN_OUT_BIN_SPLIT;
    }
  }
  if (do_nfg){
    o = o | TN_OUT_LUA;
  }



  cout << rang::style::reset << rang::fg::green <<"----Parsing JSON file " << filename << "----\n" << rang::style::reset;
  //Test crossbar logic
  //char *filename = "/Users/markplagge/dev/NeMo/scripts/model/th_corelet_net.json";
    
  TN_Main model = create_tn_data(filename);
  cout << "\n Created Model. \n";
  TN_Output out(save_file,model,o );
  cout << "\n saving json file  to " << save_file << "\n";
  out.write_data();

//#endif

return 0;
}
