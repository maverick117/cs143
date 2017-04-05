#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#include <map>
#include <list>
#include <vector>


#define TRUE 1
#define FALSE 0

class ClassTable;

typedef ClassTable *ClassTableP;

typedef std::vector<Symbol> Signature;

typedef std::map<Symbol, Signature> FunctionEnvironment;
typedef SymbolTable<Symbol, Entry> ObjectEnvironment;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  std::ostream& error_stream;
  std::map<Symbol, Symbol> inheritanceTree;
  void findLoops(Symbol origClass, Symbol s, std::map<Symbol, bool> history);
  std::map<Symbol, std::pair<ObjectEnvironment, FunctionEnvironment>> environment;
  std::map<Symbol, Class_> classTable;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  std::ostream& semant_error();
  std::ostream& semant_error(Class_ c);
  std::ostream& semant_error(Symbol filename, tree_node *t);
  bool isTypeLessThan(Symbol T1, Symbol T2);
  Symbol commonAncestor(Symbol T1, Symbol T2);
  Symbol commonAncestor(std::vector<Symbol> const& symbols);


  Class_ getClass(Symbol cname) {return classTable[cname];}
  Signature& M(Symbol C, Symbol F) {return environment[C].second[F]; }
  ObjectEnvironment& O(Symbol C) { return environment[C].first; }
  bool findMethod(Symbol classname, Symbol fname){return environment[classname].second.find(fname) != environment[classname].second.end();}
  bool findClass(Symbol classname){return classTable.find(classname) != classTable.end();}
};

#endif

