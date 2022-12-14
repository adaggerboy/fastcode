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
  bool unsettable, getbylink, internal, construct;
  unsigned int attribute;
  string attr;
};

struct Method {
  string type;
  string name;
  string defreturn;
  int vision;
  bool constant, virtuals, pure, inlines, statics;
  list<Field> args;
  // bool unsettable, getbylink, internal, construct;
  // unsigned int attribute;
  // string attr;
};

Field parseField(string f) {
  Field res;
  int t = f.find_first_of(':');
  int e = f.find_first_of('=');
  int a = f.find_first_of('@');
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
    res.construct = false;
    while (1) {
      if(f[i] == '~') {
        if(res.internal) {
          res.internal = false;
          res.unsettable = true;
        }
        else res.internal = true;
      } else if(f[i] == '&') res.getbylink = true;
      else if(f[i] == '>') res.construct = true;
      else break;
      i++;
    }
    res.type = f.substr(i, t - i);
    if(res.type.find("const ") + 1) res.unsettable = true;
    if(a + 1) {
      if(f[a + 1] == 'M') {
        res.attr = f.substr(a + 2, f.npos);
        res.attribute = 0x80000001;
      } else if(f[a + 1] == 'n') {
        res.attr = f.substr(a + 2, f.npos);
        res.attribute = 0x80000002;
      } else if(f[a + 1] == 'm') {
        res.attr = f.substr(a + 2, f.npos);
        res.attribute = 0x80000003;
      }
    } else a = f.length();
    if(e + 1) {
      res.name = f.substr(t + 1, e - t - 1);
      res.init = f.substr(e + 1, a - e - 1);
    } else res.name = f.substr(t + 1, a - t - 1);
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
    res.construct = false;
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
      else if(f[i] == '>') res.construct = true;
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
    res.attribute = 0;
    if(a + 1) {
      if(f[a + 1] == 'M') {
        res.attr = f.substr(a + 2, f.npos);
        res.attribute = 0x80000001;
      } else if(f[a + 1] == 'n') {
        res.attr = f.substr(a + 2, f.npos);
        res.attribute = 0x80000002;
      } else if(f[a + 1] == 'm') {
        res.attr = f.substr(a + 2, f.npos);
        res.attribute = 0x80000003;
      }
    } else a = f.length();
    if(e + 1) {
      res.name = f.substr(i, e - i);
      res.init = f.substr(e + 1, a - e - 1);
    } else res.name = f.substr(i, a - i);
  }
  return res;
}

Method parseMethod(string s) {
  Method res;
  string f;
  int cntr = 0;
  string tmp = "";
  for (int i = 0; i < s.length(); i++) {
    if(s[i] == '[') {
      if(!cntr++) continue;
    }
    else if(s[i] == ']') cntr--;
    if(!cntr) {
      if(tmp.length()) {
        res.args.push_back(parseField(tmp));
        tmp.clear();
      }
      if(s[i] != ']') f += s[i];
    } else if (cntr == 1 && s[i] == ','){
      res.args.push_back(parseField(tmp));
      tmp.clear();
    } else tmp += s[i];
  }

  int t = f.find_first_of(':');
  int e = f.find_first_of('=');
  int a = f.find_first_of('@');

  if(t + 1) {
    int i = 0;
    if(f[i] == '!') res.vision = 0;
    else if(f[i] == '+') res.vision = 2;
    else {
      res.vision = 1;
      i--;
    }
    i++;
    res.virtuals = false;
    res.inlines = false;
    res.constant = false;
    res.pure = false;
    res.statics = false;
    while (1) {
      if(f[i] == '?') {
        if(res.virtuals) {
          res.pure = true;
        }
        else res.virtuals = true;
      } else if(f[i] == '~') res.constant = true;
      else if(f[i] == '>') res.inlines = true;
      else if(f[i] == '&') res.statics = true;
      else break;
      i++;
    }
    res.type = f.substr(i, t - i);
    if(a + 1) {
      // if(f[a + 1] == 'M') {
      //   res.attr = f.substr(a + 2, f.npos);
      //   res.attribute = 0x80000001;
      // } else if(f[a + 1] == 'n') {
      //   res.attr = f.substr(a + 2, f.npos);
      //   res.attribute = 0x80000002;
      // } else if(f[a + 1] == 'm') {
      //   res.attr = f.substr(a + 2, f.npos);
      //   res.attribute = 0x80000003;
      // }
    } else a = f.length();
    if(e + 1) {
      res.name = f.substr(t + 1, e - t - 1);
      res.defreturn = f.substr(e + 1, a - e - 1);
    } else res.name = f.substr(t + 1, a - t - 1);
  } else {
    int i = 0;
    if(f[i] == '+') res.vision = 2;
    else if(f[i] == '!') res.vision = 0;
    else {
      res.vision = 1;
      i--;
    }
    i++;
    res.virtuals = false;
    res.constant = false;
    res.pure = false;
    while (1) {
      if(f[i] == 'C') {
        res.type += "const ";
      }
      else if(f[i] == 'V') res.type += "volatile ";
      else if(f[i] == 'S') res.type += "static ";
      else if(f[i] == 'u') res.type += "unsigned ";
      else if(f[i] == '?') {
        if(res.virtuals) {
          res.pure = true;
        }
        else res.virtuals = true;
      } else if(f[i] == '~') res.constant = true;
      else if(f[i] == '>') res.inlines = true;
      else if(f[i] == '&') res.statics = true;
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
    if(a + 1) {
      // if(f[a + 1] == 'M') {
      //   res.attr = f.substr(a + 2, f.npos);
      //   res.attribute = 0x80000001;
      // } else if(f[a + 1] == 'n') {
      //   res.attr = f.substr(a + 2, f.npos);
      //   res.attribute = 0x80000002;
      // } else if(f[a + 1] == 'm') {
      //   res.attr = f.substr(a + 2, f.npos);
      //   res.attribute = 0x80000003;
      // }
    } else a = f.length();
    if(e + 1) {
      res.name = f.substr(i, e - i);
      res.defreturn = f.substr(e + 1, a - e - 1);
    } else res.name = f.substr(i, a - i);
  }
  return res;
}

list<string> genFunction(Method f, bool def, list<string> inside) {
  list<string> res;
  string tmp;
  if(f.virtuals) tmp += "virtual ";
  if(f.inlines) tmp += "inline ";
  if(f.statics) tmp += "static ";
  tmp += f.type + " " + f.name + "(";
  bool nfirst = false;
  for (Field ff : f.args) {
    if(nfirst) tmp += ", ";
    tmp += ff.type + " " + ff.name;
    nfirst = true;
  }
  tmp += ")";
  if(f.constant) tmp += " const";
  if(f.pure) {
    tmp += " = 0;";
    res.push_back(tmp);
  } else if(def) {
    tmp += " {";
    res.push_back(tmp);
    tmp.clear();
    for (string s : inside) {
      res.push_back("\t" + s);
    }
    res.push_back("\t");
    if(f.defreturn.length()) {
      tmp += "\treturn " + f.defreturn + ";";
      res.push_back(tmp);
      tmp.clear();
    }
    res.push_back("}");
  } else {
    tmp += ';';
    res.push_back(tmp);
  }
  return res;
}

list<string> genClass(string s, bool def) {
  list<string> res;
  string fieldsp = "";
  string methodsp = "";
  string classname = "";

  int mode = 0;
  int cntr = 1;

  list<Field> fields;
  list<Method> methods;

  for (int i = 0; i < s.length(); i++) {
    if(mode == 0) {
      if(isalpha(s[i])) classname += s[i];
      else {
        mode = 1;
        i--;
        continue;
      }
    } else if (mode == 1) {
      if(s[i] == '[') {
        mode = 2;
        cntr = 1;
      }
    } else if (mode == 2) {
      if(s[i] == '[') cntr++;
      else if(s[i] == ']') cntr--;
      if(!cntr) {
        if(fieldsp.length()) {
          fields.push_back(parseField(fieldsp));
          fieldsp.clear();
        }
        mode = 3;
      } else if (cntr == 1 && s[i] == ','){
        fields.push_back(parseField(fieldsp));
        fieldsp.clear();
      } else fieldsp += s[i];
    } else if (mode == 3) {
      if(s[i] == '[') {
        mode = 4;
        cntr = 1;
      }
    } else if (mode == 4) {
      if(s[i] == '[') cntr++;
      else if(s[i] == ']') cntr--;
      if(!cntr) {
        if(methodsp.length()) {
          methods.push_back(parseMethod(methodsp));
          methodsp.clear();
        }
        mode = 5;
      } else if (cntr == 1 && s[i] == ','){
        methods.push_back(parseMethod(methodsp));
        methodsp.clear();
      } else methodsp += s[i];
    }
  }


  // int fieldb = 0;
  // int fielde;
  // if (fieldsp.length()) while(1) {
  //   fielde = fieldsp.find_first_of(',', fieldb);
  //   if(fielde + 1) {
  //     fields.push_back(parseField(fieldsp.substr(fieldb, fielde - fieldb)));
  //     fieldb = fielde + 1;
  //   } else {
  //     fields.push_back(parseField(fieldsp.substr(fieldb, fielde == s.npos ? fielde : (fielde - fieldb))));
  //     break;
  //   }
  // }
  // fieldb = 0;
  // if (methodsp.length()) while(1) {
  //   fielde = methodsp.find_first_of(',', fieldb);
  //   if(fielde + 1) {
  //     methods.push_back(parseMethod(methodsp.substr(fieldb, fielde - fieldb)));
  //     fieldb = fielde + 1;
  //   } else {
  //     methods.push_back(parseMethod(methodsp.substr(fieldb, fielde == s.npos ? fielde : (fielde - fieldb))));
  //     break;
  //   }
  // }

  res.push_back("class " + classname + " {");
  res.push_back("private:");
  for (Field f : fields) {
    if(f.vision == 0) {
      string field = "\t" + f.type + " " + f.name;
      if(f.init.length() && !f.construct) field += " = " + f.init;
      field += ';';
      res.push_back(field);
    }
  }
  for (Method f : methods) {
    if(f.vision == 0) {
      list<string> sg = genFunction(f, def, list<string>());
      for(string ss : sg) {
        res.push_back("\t" + ss);
      }
    }
  }

  res.push_back("protected:");
  for (Field f : fields) {
    if(f.vision == 1) {
      string field = "\t" + f.type + " " + f.name;
      if(f.init.length() && !f.construct) field += " = " + f.init;
      field += ';';
      res.push_back(field);
    }
  }
  for (Method f : methods) {
    if(f.vision == 1) {
      list<string> sg = genFunction(f, def, list<string>());
      for(string ss : sg) {
        res.push_back("\t" + ss);
      }
    }
  }

  res.push_back("public:");
  for (Field f : fields) {
    if(f.vision == 2) {
      string field = "\t" + f.type + " " + f.name;
      if(f.init.length() && !f.construct) field += " = " + f.init;
      field += ';';
      res.push_back(field);
    }
  }
  {
    string tmp = "\t" + classname + "(";
    string tmptmp;
    bool c = false;
    for (Field f : fields) if(f.construct && !f.init.length()) {
      c = true;
      if(tmptmp.length()) tmp += tmptmp + ", ";
      tmptmp = f.type + " _" + f.name;
    }
    for (Field f : fields) if(f.construct && f.init.length()) {
      c = true;
      if(tmptmp.length()) tmp += tmptmp + ", ";
      tmptmp = f.type + " _" + f.name + " = " + f.init;
    }
    tmp += tmptmp + ')';
    if(def) {
      if(c) {
        tmp += " : ";
        tmptmp = "";
        for (Field f : fields) if(f.construct) {
          if(tmptmp.length()) tmp += tmptmp + ", ";
          tmptmp = f.name + "(_" + f.name + ")";
        }
        tmp += tmptmp;
      }
      tmp += " {";
      res.push_back(tmp);
      for (Field f : fields) if(f.attribute & 0x80000000) {
        tmp = "\t\t";
        if((f.attribute & 0x7fffffff) == 1) {
          if(f.type.find_last_of('*') == f.type.npos) Error("allocation of non-pointer type");
          tmp += f.name + " = ";
          if(f.type != "void*") tmp += "(" + f.type + ")";
          tmp += "malloc(" + f.attr + ");";
        } else if((f.attribute & 0x7fffffff) == 3) {
          if(f.type.find_last_of('*') == f.type.npos) Error("allocation of non-pointer type");
          string ntype = f.type;
          ntype.erase(ntype.find_last_of('*'));
          tmp += f.name + " = ";
          if(f.type != "void*") tmp += "(" + f.type + ")";
          tmp += "malloc(";
          if(f.attr.length()) tmp += f.attr + " * ";
          tmp += "sizeof(" + ntype + "));";
        } else if((f.attribute & 0x7fffffff) == 2) {
          if(f.type.find_last_of('*') == f.type.npos) Error("allocation of non-pointer type");
          string ntype = f.type;
          ntype.erase(ntype.find_last_of('*'));
          tmp += f.name + " = new " + ntype;
          if(f.attr.length()) tmp += "[" + f.attr + "]";
          tmp += ";";
        }
        res.push_back(tmp);
      }
      res.push_back("\t}");
    } else {
      tmp += ';';
      res.push_back(tmp);
    }
  }
  {
    string tmp = "\t~" + classname + "()";
    if(def) {
      tmp += " {";
      res.push_back(tmp);
      for (auto f = fields.end(); f != fields.begin(); f--) if(f->attribute & 0x80000000) {
        tmp = "\t\t";
        if((f->attribute & 0x7fffffff) == 1) {
          tmp += "free(" + f->name + ");";
        } else if((f->attribute & 0x7fffffff) == 3) {
          tmp += "free(" + f->name + ");";
        } else if((f->attribute & 0x7fffffff) == 2) {
          if(f->attr.length()) tmp += "delete [] ";
          else tmp += "delete ";
          tmp += f->name + ";";
        }
        res.push_back(tmp);
      }
      res.push_back("\t}");
    } else {
      tmp += ";";
      res.push_back(tmp);
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
        tmp += "\t\treturn " + f.name + ";";
        res.push_back(tmp);
        tmp.clear();
        res.push_back("\t}");
      } else {
        tmp += ';';
        res.push_back(tmp);
      }

      if(!f.unsettable && !f.getbylink) {
        tmp = "\tvoid set" + infname + "(" + f.type + " value)";
        if(def) {
          tmp += " {";
          res.push_back(tmp);
          tmp.clear();
          tmp += "\t\t" + f.name + " = value;";
          res.push_back(tmp);
          tmp.clear();
          res.push_back("\t}");
        } else {
          tmp += ';';
          res.push_back(tmp);
        }
      }
    }
  }
  for (Method f : methods) {
    if(f.vision == 2) {
      list<string> sg = genFunction(f, def, list<string>());
      for(string ss : sg) {
        res.push_back("\t" + ss);
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
