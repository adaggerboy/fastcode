#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <list>
#include "utils.hpp"

std::string unassignableOptions = "";

using namespace std;
using namespace std::filesystem;

struct Field {
  string type;
  string name;
  string init;
  int vision;
  bool unsettable, getbylink, internal;
};

Field parseField(string f) {
  Field res;
  int t = f.find_first_of(':');
  int e = f.find_first_of('=');
  if(t + 1) {
    int i = 0;
    if(f[i] == '!') res.vision = 0;
    else if(f[i] == '+') res.vision = 2;
    else {
      res.vision = 1;
      i--;
    }
    i++;
    res.unsettable = false;
    res.internal = false;
    res.getbylink = false;
    while (1) {
      if(f[i] == '~') {
        if(res.internal) {
          res.internal = false;
          res.unsettable = true;
        }
        else res.internal = true;
      } else if(f[i] == '&') res.getbylink = true;
      else break;
      i++;
    }

    res.type = f.substr(i, t - i);
    if(res.type.find("const") + 1) res.unsettable = true;
    if(e + 1) {
      res.name = f.substr(t + 1, e - t - 1);
      res.init = f.substr(e + 1, f.npos);
    } else res.name = f.substr(t + 1, f.npos);
  } else {
    int i = 0;
    if(f[i] == '+') res.vision = 2;
    else if(f[i] == '!') res.vision = 0;
    else {
      res.vision = 1;
      i--;
    }
    i++;
    res.unsettable = false;
    res.internal = false;
    res.getbylink = false;
    while (1) {
      if(f[i] == 'C') {
        res.type += "const ";
        res.unsettable = true;
      }
      else if(f[i] == 'V') res.type += "volatile ";
      else if(f[i] == 'S') res.type += "static ";
      else if(f[i] == 'u') res.type += "unsigned ";
      else if(f[i] == '~') {
        if(res.internal) {
          res.internal = false;
          res.unsettable = true;
        }
        else res.internal = true;
      } else if(f[i] == '&') res.getbylink = true;
      else break;
      i++;
    }

    if(f[i] == 'i') res.type += "int";
    else if(f[i] == 'c') res.type += "char";
    else if(f[i] == 's') res.type += "short";
    else if(f[i] == 'l') res.type += "long";
    else if(f[i] == 'L') res.type += "long long";
    else if(f[i] == 'f') res.type += "float";
    else if(f[i] == 'd') res.type += "double";
    else if(f[i] == 'D') res.type += "long double";
    else if(f[i] == 'v') res.type += "void";
    else Error("invalid field type");
    i++;
    while(f[i] == '*' || f[i] == '&') {
      res.type += f[i++];
    }
    if(e + 1) {
      res.name = f.substr(i, e - i);
      res.init = f.substr(e + 1, f.npos);
    } else res.name = f.substr(i, f.npos);
  }
  return res;
}

list<string> genClass(string s, bool def) {
  list<string> res;
  int specbegin = s.find_first_of('[');
  int specend = s.find_first_of(']');
  string classname = s.substr(0, specbegin);
  string fieldsp = s.substr(specbegin + 1, specend - specbegin - 1);
  list<Field> fields;
  int fieldb = 0;
  int fielde;
  while(1) {
    fielde = fieldsp.find_first_of(',', fieldb);
    if(fielde + 1) {
      fields.push_back(parseField(fieldsp.substr(fieldb, fielde - fieldb)));
      fieldb = fielde + 1;
    } else {
      fields.push_back(parseField(fieldsp.substr(fieldb, fielde == s.npos ? fielde : (fielde - fieldb))));
      break;
    }
  }
  res.push_back("class " + classname + " {");
  res.push_back("private:");
  for (Field f : fields) {
    if(f.vision == 0) {
      string field = "\t" + f.type + " _" + f.name;
      if(f.init.length()) field += " = " + f.init;
      field += ';';
      res.push_back(field);
    }
  }
  res.push_back("protected:");
  for (Field f : fields) {
    if(f.vision == 1) {
      string field = "\t" + f.type + " _" + f.name;
      if(f.init.length()) field += " = " + f.init;
      field += ';';
      res.push_back(field);
    }
  }
  res.push_back("public:");
  for (Field f : fields) {
    if(f.vision == 2) {
      string field = "\t" + f.type + " " + f.name;
      if(f.init.length()) field += " = " + f.init;
      field += ';';
      res.push_back(field);
    }
  }

  for (Field f : fields) {
    if(f.vision == 1) {
      if(f.internal) continue;
      string infname = f.name;
      if(isalpha(infname[0])) infname[0] = toupper(infname[0]);

      string tmp = "\t" + f.type;
      if(f.getbylink) tmp += '&';
      tmp += " get" + infname + "()";
      if(!f.getbylink) tmp += " const";
      if(def) {
        tmp += " {";
        res.push_back(tmp);
        tmp.clear();
        tmp += "\t\t return _" + f.name + ";";
        res.push_back(tmp);
        tmp.clear();
        res.push_back("\t}");
      } else {
        tmp += ';';
        res.push_back(tmp);
      }

      if(!f.unsettable && !f.getbylink) tmp = "\tvoid set" + infname + "(" + f.type + " value)";
      if(def) {
        tmp += " {";
        res.push_back(tmp);
        tmp.clear();
        tmp += "\t\t _" + f.name + " = value;";
        res.push_back(tmp);
        tmp.clear();
        res.push_back("\t}");
      } else {
        tmp += ';';
        res.push_back(tmp);
      }
    }
  }
  res.push_back("};");
  return res;
}

list<string> parseExpr(string s) {
  int mode = 0;
  int i = 0;
  if(s[i] == 'c') {
    return genClass(s.substr(i + 1, s.npos), false);
  } else if(s[i] == 'C') {
    return genClass(s.substr(i + 1, s.npos), true);
  }

}

bool detectFile(string s) {
  string tmp;
  fstream file(s, ios::in | ios::out);
  while(getline(file, tmp)) {
    while(tmp.length() && isspace(tmp[0])) tmp.erase(0, 1);
    if(tmp[0] == '$') return true;
  }
  return false;
}

void output(list<string>& flist, string path = ".") {
  for (const auto& entry : directory_iterator(path)) {
    if(entry.path().filename().string()[0] == '.') continue;
    if(entry.is_directory()) output(flist, entry.path().string());
    else {
      string ext = entry.path().extension().string();
      if(ext == ".cpp" || ext == ".hpp" || ext == ".cc" || ext == ".c" || ext == ".h" || ext == ".tcc")
        flist.push_back(entry.path().string());
    }
  }

}

void parseFile(string s) {
  ifstream filer(s, ios::in);
  ofstream backup(s + ".bkp", ios::out | ios::trunc);
  backup << filer.rdbuf();
  filer.close();
  backup.close();
  ofstream file(s, ios::out | ios::trunc);
  ifstream ref(s + ".bkp", ios::in);
  string tmp;
  string pref;
  list<string> macro;
  while(getline(ref, tmp)) {
    int pos = tmp.find_first_of('$');
    if(pos + 1) {
      pref = tmp.substr(0, pos);
      macro = parseExpr(tmp.substr(pos + 1, tmp.npos));
      for(string s : macro) {
        file << pref << s << endl;
      }
    } else {
      file << tmp << endl;
    }
  }


}

int main(int argc, char **argv) {

  CommandParseResult args = ParseArguments(argc, argv);

  int mode = 0;

  list<string> files;
  string expr;

  for (int i = 0; i < args.options.size(); i++) {
    if(!mode) {
      if(args.options[i] == 'e') {
        if(args.optionArgs.count('e')) {
          expr = args.optionArgs['e'];
          mode = 1;
        } else Error("unspecified expression");
      }
    }

  }

  if(mode == 0) {
    if(args.args.size() > 0) {
      files = args.args;
    } else {
      output(files, ".");
    }

    for(string f : files) {
      // std::cout << f << '\n';
      if(detectFile(f)) {
        parseFile(f);
      }
    }

    std::cout << files.size() << '\n';
  } else if(mode == 1) {
    list<string> a = parseExpr(expr);
    for(string s : a) {
      cout << s << '\n';
    }
  }


  return 0;
}
