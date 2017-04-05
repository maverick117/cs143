class C {
	a : Int <- 4+d;
    d : Int <- 5+a;
	b : Bool;
	init(x : Int, y : Bool) : C {
           {
        5.copy();
		b <- y;
		self;
           }
	};
};

Class Main {
    c: Int;
	main():C {
	  (new C).init(1,true)
	};
};
