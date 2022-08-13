#ifndef __LEX_HPP
#define __LEX_HPP

string keywords[] = {
  "struct",
  "class",
  "for",
  "if",
  "while",
  "else",
  "private",
  "public",
  "protected",
  "virtual",
  "friend",
  "switch",
  "extern",
  "continue",
  "case",
  "break",
  "try",
  "catch",
  "const",
  "volatile",
  "static",
  "noexcept",
  "using",
  "namespace",
  "template",
  "auto",
  "default",
  "do",
  "enum",
  "union",
  "inline",
  "goto",
  "mutable",
  "register",
  "return",
  "throw",
  "typedef",
  "typeid",
  "constexpr"
};


string operators[] = {
  "+",
  "-",
  "/",
  "*",
  "==",
  "+=",
  "-=",
  "*=",
  "/=",
  "!",
  "~",
  "%",
  "^",
  "&",
  "|",
  "&&",
  "||",
  "++",
  "--",
  ".",
  "::",
  "->",
  ">",
  "<",
  ">=",
  "<=",
  "<<",
  ">>",
  "&=",
  "|=",
  "!=",
  "?",
  ":"
};

string delimeters[] = {
  "[",
  "]",
  "{",
  "}",
  ",",
  ";",
  "(",
  ")",
  "<",
  ">",
};

struct token {
  string content;
  enum type {keyword, literal, operator, delimeter, label} type;
};

#endif /* end of include guard: __LEX_HPP */
