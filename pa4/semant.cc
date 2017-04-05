

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>
#include <map>

#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val,
	False,
	True;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
    True        = idtable.add_string("True");
    False       = idtable.add_string("False");
}



void ClassTable::findLoops(Symbol orig, Symbol s, std::map<Symbol, bool> history)
{
	auto parent = classTable[s]->get_parent();

	if (parent == SELF_TYPE){
		semant_error(classTable[s])<<"Cannot inherit from SELF_TYPE."<<endl;
		parent = Object;
	} else if(parent==Int){
		semant_error(classTable[s])<<"Cannot inherit from Int"<<endl;;
		parent = Object;
	} else if(parent==Bool){
		semant_error(classTable[s])<<"Cannot inherit from Bool"<<endl;;
		parent = Object;
	} else if (parent==Str){
		semant_error(classTable[s])<<"Cannot inherit from String"<<endl;;
		parent = Object;

	} else if (!findClass(parent) && s!=Object){
		semant_error(classTable[s])<<"Parent class of "<<s<< " not found"<<endl;
		parent = Object;
	}


	if (history[parent] == true){
		semant_error(classTable[s])<<"Found a loop in the inheritance tree."<<endl;;
		return;
	}

	if (parent == No_class){
		parent = Object;
	}

	if (classTable.find(parent) == classTable.end()) {
		semant_error(classTable[s])<<"Class definition not found."<<endl;;
		return;
	}

	Features features = classTable[s]->get_features();
	for (int j = features->first(); features->more(j); j = features->next(j)){
		Feature feature = features->nth(j);
		feature->add(this, orig);
	}

	// Once at the top of the hierarchy we are done
	if (s == Object){
		return;
	}

	// Recursive step. Use history as a map to detect loops
	history[s] = true;
	findLoops(orig, parent, history);
}


ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) 
{
    install_basic_classes();
    bool isMainDefined = false;

    // Form the class table: for each class name, add the corresponding class pointer
    for(int i = classes->first(); classes->more(i); i = classes->next(i)){
    	Class_ c = classes->nth(i);
    	Symbol className = c->get_name();
    	if (className == Main){
    		isMainDefined = true;
    	}
    	Symbol parent = c->get_parent();
    	if (className==Int){
    		semant_error(c)<<"Cannot declare Int"<<endl;
    	} else if (className == Bool){
    		semant_error(c)<<"Cannot declare Bool"<<endl;
    	} else if (className == Str){
    		semant_error(c)<<"Cannot declare String"<<endl;
    	} else if (className == SELF_TYPE){
    		semant_error(c)<<"Cannot declare SELF_TYPE"<<endl;
    	} else if (className == Object){
    		semant_error(c)<<"Cannot declare Object"<<endl;
    	} else if (findClass(className)){
    		semant_error(c)<<"Redefinition of class "<<className<<endl;
    	} else{
    		classTable[className] = c;
    	}
    }

    if (!isMainDefined){
		semant_error()<<"Class Main is not defined."<<endl;
    }

    for (auto c : classTable){

    	// Start adding the self pointer to each class
    	environment[c.first].first.enterscope();
    	environment[c.first].first.addid(self, SELF_TYPE);

    	// Detect loops in the class hierarchy, and add the proper signatures
    	// to each class
    	findLoops(c.first, c.first, std::map<Symbol, bool>());
    }
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
    // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    classTable[Object] = Object_class;

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  
    classTable[IO] = IO_class;

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);
    classTable[Int] = Int_class;

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);
    classTable[Bool] = Bool_class;

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);
    classTable[Str] = Str_class;
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

bool ClassTable::isTypeLessThan(Symbol T1, Symbol T2)
{
	if (T1 == T2){
		return true;
	}
	if (T2==Object){
		return true;
	}

	if (T1==Bool || T2 == Bool )
		return false;
	if (T1==Int || T2 == Int)
		return false;
	if (T1==Str || T2 == Str)
		return false;

	Symbol current = classTable[T1]->get_name();
	while (current != T2 && current != No_class && current != Object){
		current = classTable[current]->get_parent();
	}
	return current != Object && current != No_class;
}

Symbol ClassTable::commonAncestor(Symbol T1, Symbol T2)
{
	if (isTypeLessThan(T1, T2)){
		return T2;
	} else if(isTypeLessThan(T2, T1)){
		return T1;
	}
	Symbol Tcurrent = classTable[T1]->get_parent();

	while (Tcurrent != Object){
		if (isTypeLessThan(T2, Tcurrent)){
			return Tcurrent;
		}
		Tcurrent = classTable[Tcurrent]->get_parent();
	}
	return Object;
}

Symbol ClassTable::commonAncestor(std::vector<Symbol> const& symbols)
{
	Symbol T = symbols[0];
	for (size_t i = 1; i <symbols.size(); i++){
		Symbol tmp = commonAncestor(T, symbols[i]);
		if (tmp == Object) return Object;
		T = tmp;
	}

	return T;
}

void method_class::add(ClassTableP ct, Symbol classname)
{

	// Given the method name&signature, check if we need to add the method from the parent class
	if (ct->findMethod(classname, name)){
		bool sameSignature(true);

		// Fill up the obejct/method environment of the leaf class
		Signature sign = ct->M(classname, name);
		if (formals->len() != int(sign.size()-1)){
			ct->semant_error(ct->getClass(classname))<<"Overriding must have same number of inputs"<<endl;
		}

		for (size_t i = formals->first(); formals->more(i); i = formals->next(i)){
			if (formals->nth(i)->getType() != sign[i]) {sameSignature = false; break;}
		}

		if (!sameSignature){
			ct->semant_error(ct->getClass(classname))<<"Overriding must have same parameter types"<<endl;
		}
	} else {
		auto& funEnv = ct->M(classname, name);
		for (int i = formals->first(); formals->more(i); i = formals->next(i)){
			Symbol signType = formals->nth(i)->getType();
			if (signType==SELF_TYPE){
				ct->semant_error(ct->getClass(classname))<<"SELF_TYPE as parameter name"<<endl;
				continue;
			}
			funEnv.push_back(signType);
		}

		if (!ct->findClass(return_type) && return_type != SELF_TYPE){
			ct->semant_error(ct->getClass(classname))<<return_type<<" not found"<<endl;
		}

		funEnv.push_back(return_type);
	}

}

void attr_class::add(ClassTableP ct, Symbol classname)
{
	if (ct->O(classname).lookup(name)!=NULL){
		ct->semant_error(ct->getClass(classname))<<"Attribute redefinition"<<endl;
	} else {
		ct->O(classname).addid(name, type_decl);
	}
}

void object_class::semant(ClassTableP ct, Symbol classname)
{
	Symbol type = ct->O(classname).lookup(name);

	if (type == NULL){
		ct->semant_error(ct->getClass(classname))<<"Object "<<name<<" not found"<<endl;
		this->type = Object;
		return;
	}
	this->type = type;
}

void assign_class::semant(ClassTableP ct, Symbol classname)
{
	if (name==self){
		ct->semant_error(ct->getClass(classname))<<"cannot assign to self"<<endl;
	}
	expr->semant(ct, classname);
	Symbol T = ct->O(classname).lookup(name);

	if (T == NULL){
		ct->semant_error(ct->getClass(classname))<<"Object "<<name<<" not found"<<endl;
		return;
		type = Object;
	}

	Symbol T1 = expr->get_type();

	if (ct->isTypeLessThan(T1, T)){
		type = T1;
	} else {
		ct->semant_error(ct->getClass(classname))<<"Type " << T1 << "does not inherit from " << T <<endl;;
		return;
	}
}

void int_const_class::semant(ClassTableP ct, Symbol classname)
{
	Symbol Int = idtable.lookup_string("Int");
	type = Int;
}

void bool_const_class::semant(ClassTableP ct, Symbol classname)
{
	Symbol Bool = idtable.lookup_string("Bool");
	type = Bool;
}

void string_const_class::semant(ClassTableP ct, Symbol classname)
{
	Symbol Str = idtable.lookup_string("String");
	type = Str;
}


// From here we specify semant methods for all the classes


void static_dispatch_class::semant(ClassTableP ct, Symbol classname)
{
	expr->semant(ct, classname);
	Symbol T0 = expr->get_type();
	Symbol T = type_name;

	if (!ct->isTypeLessThan(T, T0)){
		ct->semant_error(ct->getClass(classname))<<"Dispatch error"<<endl;;
		type = Object;
		return;
	}

	for(int ii = actual->first(); actual->more(ii); ii = actual->next(ii)){
		actual->nth(ii)->semant(ct, classname);
	}

	if (!ct->isTypeLessThan(T0, classname)){
		ct->semant_error(ct->getClass(classname))<<"Dispatch error"<<endl;
		type = Object;
		return;
	}

	size_t numInputs = actual->len() - 1;
	if (numInputs != size_t(actual->len())){
		ct->semant_error(ct->getClass(classname))<<"Wrong number of inputs"<<endl;;
		type = Object;
		return;
	}

	// Note that it is T and not T0 (but we previously checked that T0 <= T)
	std::vector<Symbol> S = ct->M(T, name);
	for (size_t j = 0;j < numInputs; j++){
		Symbol signType = S[j];
		Symbol callType = actual->nth(j)->get_type();
		if (!ct->isTypeLessThan(callType, signType)){
			ct->semant_error(ct->getClass(classname))<<"Static Dispatch error: "<< callType << " is not conform to "<<signType<<endl;;
			return;
		}
	}
	Symbol T1first = S.back(); // T_(n+1)'
	Symbol T1 = (T1first==SELF_TYPE? T0 : T1first); // T_(n+1)

	type = T1;
}

void dispatch_class::semant(ClassTableP ct, Symbol C)
{
	expr->semant(ct, C);
	// e0 type
	Symbol T0 = expr->get_type();
	Symbol T0first = (T0==SELF_TYPE? C : T0);

	// Determine the type of the e1...en
	for(int ii = actual->first(); actual->more(ii); ii = actual->next(ii)){
		actual->nth(ii)->semant(ct, C);
	}
	// Check that they match with the method signature
	std::vector<Symbol> T = ct->M(T0first, name);
	if (!T.size()){
		ct->semant_error(ct->getClass(C))<<"Unkown function call "<<name<<endl;;
		type = Object;
		return;
	}

	// Number of expected inputs
	size_t numInputs = T.size() - 1;

	if (numInputs != size_t(actual->len())){
		ct->semant_error(ct->getClass(C))<<"Wrong number of inputs"<<endl;;
		type = Object;
		return;
	}

	// Check T_i <= T_i'
	size_t j = 0;
	for (j = 0;j <	 numInputs; j++){
		Symbol signType = T[j];

		Symbol callType = actual->nth(j)->get_type();
		if (callType == SELF_TYPE) callType = C;

		if (!ct->isTypeLessThan(callType, signType)){
			ct->semant_error(ct->getClass(C))<<"Dispatch error: "<< callType << " is not conform to "<<signType<<endl;;
			type = Object;
			return;
		}
	}

	Symbol T1first = T.back(); // T_(n+1)'
	Symbol T1 = (T1first==SELF_TYPE? T0 : T1first); // T_(n+1)

	// Assign the type
	type = T1;
}

void branch_class::semant(ClassTableP ct, Symbol classname)
{
	ct->O(classname).enterscope();
	ct->O(classname).addid(name, type_decl);
	expr->semant(ct, classname);
	ct->O(classname).exitscope();
}

void cond_class::semant(ClassTableP ct, Symbol classname)
{
	pred->semant(ct, classname);
	then_exp->semant(ct, classname);
	else_exp->semant(ct, classname);

	type = ct->commonAncestor(then_exp->get_type(), else_exp->get_type());
}

void block_class::semant(ClassTableP ct, Symbol classname)
{
	int i = 0;

	for (i = body->first(); body->more(i); i = body->next(i)){
		body->nth(i)->semant(ct, classname);
	}

	// Last expression in the block determines the type
	type = body->nth(i-1)->get_type();
}

void let_class::semant(ClassTableP ct, Symbol classname)
{
	Symbol T0  = type_decl;
	Symbol T0first = (T0==SELF_TYPE? classname : T0);

	init->semant(ct, classname);


	Symbol T1 = init->get_type();
	if (T1==SELF_TYPE){T1 = classname;}
	if (T1 != No_type){
		if (!ct->isTypeLessThan(T1, T0first)){
			ct->semant_error(ct->getClass(classname))<<"Let error"<<endl;;
			type = Object;
			return;
		}
	}


	ct->O(classname).enterscope();
	if (identifier==self){
		ct->semant_error(ct->getClass(classname))<<"Self cannot be let-initialized"<<endl;;
	} else {
		ct->O(classname).addid(identifier, T0);
	}

	body->semant(ct, classname);
	Symbol T2 = body->get_type();

	ct->O(classname).exitscope();

	type = T2;

}

void typcase_class::semant(ClassTableP ct, Symbol classname)
{
	std::vector<Symbol> symbols;
	for (int i = cases->first(); cases->more(i); i = cases->next(i)){
		cases->nth(i)->semant(ct, classname);
		Symbol branchType = cases->nth(i)->get_type();
		if (std::find(symbols.begin(), symbols.end(),branchType)!= symbols.end()){
			ct->semant_error(ct->getClass(classname))<<"Branch type "<<branchType<<"defined twice"<<endl;;
		}
		symbols.push_back(branchType);
	}
	expr->semant(ct, classname);
	type = ct->commonAncestor(symbols);
}

void loop_class::semant(ClassTableP ct, Symbol classname){
	pred->semant(ct, classname);
	body->semant(ct, classname);
	Symbol Bool = idtable.lookup_string("Bool");
	Symbol Object = idtable.lookup_string("Object");
	if(pred->get_type()!=Bool){
		ct->semant_error(ct->getClass(classname))<<"Loop predicate must be bool"<<endl;;

	}
	type = Object;
}

void isvoid_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	Symbol Bool = idtable.lookup_string("Bool");
	type = Bool;
}


void comp_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	if (e1->get_type() != Bool){
		ct->semant_error(ct->getClass(classname))<<"Complement of a non-bool value"<<endl;;
	}
	type = Bool;
}


bool cmpShared(Expression e1, Expression e2){

	if ( e1->get_type() == Int && (e2->get_type()==Bool || e2->get_type()==Str)){
		return false;
	} else if (e1->get_type()==Str &&(e2->get_type()==Int || e2->get_type()==Bool)) {
		return false;
	} else if (e1->get_type()==Bool &&( e2->get_type()==Str || e2->get_type()==Bool)){
		return false;
	} else{
		return true;
	}
}

void neg_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	if (e1->get_type() != Int){
		ct->semant_error(ct->getClass(classname))<<"Negation of non Int class"<<endl;;
	}
	type = Int;
}

void plus_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);

	if (e1->get_type()!=Int || e2->get_type() !=Int){
		ct->semant_error(ct->getClass(classname))<<"Plus takes both integer arguments"<<endl;;
	}

	type = Int;
}

void mul_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);

	if (e1->get_type()!=Int || e2->get_type() !=Int){
		ct->semant_error(ct->getClass(classname))<<"Mult takes both integer arguments"<<endl;;
	}

	type = Int;
}

void divide_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);

	if (e1->get_type()!=Int || e2->get_type() !=Int){
		ct->semant_error(ct->getClass(classname))<<"Div takes both integer arguments"<<endl;;
	}
	type = Int;

}

void sub_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);

	if (e1->get_type()!=Int || e2->get_type() !=Int){
		ct->semant_error(ct->getClass(classname))<<"Mult takes both integer arguments"<<endl;;
	}
	type = Int;
}

void eq_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);

	if (!cmpShared(e1,e2)){
		ct->semant_error(ct->getClass(classname))<<"Classes cannot be compared "<<endl;;

	}
	type = Bool;
}

void lt_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);
	Symbol Bool = idtable.lookup_string("Bool");
	if (!cmpShared(e1,e2)){
		ct->semant_error(ct->getClass(classname))<<"Classes cannot be compared "<<endl;;
	}
	type = Bool;
}

void leq_class::semant(ClassTableP ct, Symbol classname)
{
	e1->semant(ct, classname);
	e2->semant(ct, classname);

	Symbol Bool = idtable.lookup_string("Bool");
	if (!cmpShared(e1,e2)){
		ct->semant_error(ct->getClass(classname))<<"Class cannot be compared "<<endl;;
	}

	type = Bool;
}

void attr_class::semant(ClassTableP ct, Symbol classname)
{
	init->semant(ct, classname);
	Symbol initType = init->get_type();
	if (initType == No_type) return;
	if (initType == SELF_TYPE) initType =classname;
	if ( !ct->isTypeLessThan(initType, type_decl)){
		ct->semant_error(ct->getClass(classname))<<"Initialization does not match the class"<<endl;;
	}

}


void method_class::semant(ClassTableP ct, Symbol classname)
{
	ct->O(classname).enterscope();
	std::map<Symbol, bool> names;

	// Add all the formals into the current scope
	for (int i = formals->first(); formals->more(i); i = formals->next(i)){
		Symbol name = formals->nth(i)->getName();
		Symbol type = formals->nth(i)->getType();

		if (names[name]){
			ct->semant_error(ct->getClass(classname))<<"Duplicate argument name"<<endl;
		}
		if (name==self){
			ct->semant_error(ct->getClass(classname))<<"Self as formal parameter"<<endl;
		}
		names[name] = true;
		ct->O(classname).addid(name, type);
	}

	Symbol T0 = ct->M(classname, name).back();

	// Determine the body type
	expr->semant(ct, classname);

	Symbol T0first = expr->get_type();
	if (T0==SELF_TYPE && T0first!=SELF_TYPE){
		ct->semant_error(ct->getClass(classname))<<"Inferred return type " << T0first << " of method "<< name<< " does not conform to declared return type SELF_TYPE"<<endl;;
	}

	//Determine the signature output type
	Symbol toCheck1 = (T0==SELF_TYPE ? classname : T0); //method return type
	Symbol toCheck2 = (T0first==SELF_TYPE ? classname : T0first); // body type

	// Check the body returns the correct type
	if (!ct->isTypeLessThan(toCheck2, toCheck1 	 )){
		ct->semant_error(ct->getClass(classname))<<"Method declaration error "<<endl;;
	}

}

void new__class::semant(ClassTableP ct, Symbol classname)
{
	type = type_name;
}


void class__class::semant(ClassTableP ct, Symbol classname){
	for (int i = features->first(); features->more(i); i = features->next(i)){
		features->nth(i)->semant(ct, classname);
	}
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semant	ically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes); // Sorted out the loops in the inheritance tree
    for(int i = classes->first(); classes->more(i); i = classes->next(i)){
    	Class_ c = classes->nth(i);
    	if (c->get_name()==SELF_TYPE) continue;
    	if (c->get_name()==Object) continue;
    	c->semant(classtable, c->get_name());
    }

    /* some semantic analysis code may go here */
    if (classtable->errors()) {
    	cerr << "Compilation halted due to static semantic errors." << endl;
    	exit(1);
    }
}


