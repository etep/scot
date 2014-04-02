#ifndef _H_type_h
#define _H_type_h

#include <iostream>
//#include <string>

using namespace std;

/*
 * This class defines the list of types that are legal in the circuit
 * parser along with some primitive functions.
 */
class Type {
private:
    string typeName;

public :

    static Type * nmosType, *pmosType;
    static Type * gndType, *vddType, *intType, *inputType;
    static Type * subcircuitType;
    static Type * instanceType, *capType, *intCapType;

    Type( string s ) {
        typeName = s;
    }
    bool equals( Type * t );
    string getName() {
        return typeName;
    }

    /*
      void PrintToStream( ostream& out) { out << typeName; }
      friend ostream& operator<<( ostream& out, Type *t)
        { t->PrintToStream(out); return out; }

      char* GetName() {return typeName; }
     */
};

#endif
