#include <string>
#include <cstring>
#include <iostream>

#include "utils.hpp"

using namespace std;

void Error(string message) {
  cout << "Error: " << message << endl;
  exit(1);
}

CommandParseResult ParseArguments(int argc, char** argv) {

  CommandParseResult res;

  char key = 0;

  for (int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == '-') {
        if(argv[i][2]) {
          for (int j = 2; j < strlen(argv[i]); j++) {
            if(argv[i][j] == '=') {
              argv[i][j] = 0;
              res.keyArgs[string(argv[i] + 2)] = string(argv[i] + j + 1);
              break;
            }
          }
          res.keys.push_back(string(argv[i] + 2));
        }
        else Error("invalid key");
        key = 0;
      } else {
        if(argv[i][1]) {
          for (int j = 1; argv[i][j]; j++) {
            if((argv[i][1] >= 'a' && argv[i][1] <= 'z') || (argv[i][1] >= 'A' && argv[i][1] <= 'Z')) {
              res.options.push_back(argv[i][j]);
              key = argv[i][j];
              for (int k = 0; k < unassignableOptions.length(); k++) {
                if(key == unassignableOptions[k]) {
                  key = 0;
                  break;
                }
              }
            } else Error("invalid option");
          }
        } else Error("invalid option");
      }
    } else {
      if(argv[i][0]) {
        if(key) res.optionArgs[key] = string(argv[i]);
        else res.args.push_back(string(argv[i]));
      }
      else Error("invalid key");
      key = 0;
    }
  }

  return res;
}
