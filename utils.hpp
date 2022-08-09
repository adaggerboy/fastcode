#ifndef MAKEPFS_HEADER
#define MAKEPFS_HEADER

#include <string>
#include <list>
#include <vector>
#include <map>

extern std::string unassignableOptions;

struct CommandParseResult {
    std::vector<char> options;
    std::vector<std::string> keys;
    std::list<std::string> args;
    std::map<char, std::string> optionArgs;
    std::map<std::string, std::string> keyArgs;
};

CommandParseResult ParseArguments(int argc, char** argv);
void Error(std::string message);

#endif /* end of include guard: MAKEPFS_HEADER */
