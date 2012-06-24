#include "Type.hpp"

Type *Type::nmosType       = new Type( "nmos"       );
Type *Type::pmosType       = new Type( "pmos"       );
Type *Type::gndType        = new Type( "gnd"        );
Type *Type::vddType        = new Type( "vdd"        );
Type *Type::intType        = new Type( "internal"   );
Type *Type::inputType      = new Type( "input"      );
Type *Type::instanceType   = new Type( "instance"   );
Type *Type::subcircuitType = new Type( "subcircuit" );
Type *Type::capType        = new Type( "capacitor"  );
Type *Type::intCapType     = new Type( "intCap"     );

bool Type::equals( Type * t ) {
   
   return strcmp( typeName.c_str(), t->getName().c_str() ) == 0;
}
