

(*  Example cool program testing as many aspects of the code generator
    as possible.
 *)
class B inherits IO{
a : String <- "ciao\n";
disp():Object {{out_string(a);out_string("\n");}};
};

class A inherits IO{
  a : Int <- 4+2;
  sum(b: Int):Int{b+a};
  disp():Object {{ out_int(sum(10));out_string("\n");}};
  setA(x:Int) : Object {a <- x};
  getA(): Int{a};
};

class Main {
    obj : A<-new A;
    main():Object{{ 
            obj.disp(); obj.setA(14); obj.disp();
    }};
};

