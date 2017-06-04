#include <assert.h>
#include <stdio.h>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"
#include <map>
#include <functional>

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

int label_index(0);


struct StorageInfo
{
	char  *  reg;
	int offset;
	bool isTemp;

	void emit(ostream& ss);
	StorageInfo(char *r, int o, bool istemp = false):reg(r), offset(o), isTemp(istemp){}
};

typedef std::map<Symbol, StorageInfo *> Storage;

std::map<Symbol, std::map<Symbol, int> > dispatchTable;
std::map<Symbol, std::map<Symbol, int> > tempTable;
std::map<Symbol, CgenNodeP> classTable;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
	std::map<Symbol, int> classTags;

   List<CgenNode> *nds;
   ostream& str;
   int stringclasstag;
   int intclasstag;
   int boolclasstag;

// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();
   void code_prototypes();
   void code_classnametab(std::string const& tabname, std::function<void(Symbol, ostream&)> const& print);
   void code_dispatchtables();
   void code_objinitializers();
   void code_objmethods();

   void fill_class_storages();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);

public:
   CgenClassTable(Classes, ostream& str);
   void code();
   CgenNodeP root();

};


class CgenNode : public class__class {
private: 
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise
   Storage storage;
   int classTag;

public:

   void initialize_attributes(ostream& ss);
   int getNumAttributes();
   Storage& getStorage(){return storage;}

   CgenNode(Class_ c,
            Basicness bstatus, int classTag,
            CgenClassTableP class_table);

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }
   int tag(){ return classTag; }
   void code_prototype(ostream& ss);
   void code_dispatchtable(std::map<Symbol, Symbol> &methodList, Symbol classname, ostream& ss);
   void code_init(ostream& ss);
   void code_init_recursive(ostream& ss, int& n);
   void fill_storage_recursive(Storage &storage, int &n);
   void code_methods(ostream& ss);
   void count_temporaries(Symbol className);
   int init_temporaries_recursive();
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};

