# Set the default target. When you make with no arguments,
# this will be the target built.
OBJDIR     = ../obj
BINDIR     = ../bin
SHDIR      = ../../shared
SHSRCDIR   = $(SHDIR)/src
SHOBJDIR   = $(SHDIR)/obj
BIN        = $(BINDIR)/diogen

.PHONY: clean

default: dirs $(BIN)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(SHOBJDIR)

# Set up the list of source and object files
SRCS = Type.cpp               \
       Path.cpp               \
       Node.cpp               \
       Gnd.cpp                \
       Vdd.cpp                \
       Capacitor.cpp          \
       Internal.cpp           \
       Transistor.cpp         \
       Subcircuit.cpp         \
       Instance.cpp           \
       Input.cpp              \
       CircuitParser.cpp      \
       TransitionTable.cpp    \
       diogen.cpp

SHOBJECTS = SystemWrapper.o
SHOBJS = $(addprefix $(SHOBJDIR)/, $(SHOBJECTS) )
# .o versions of the c/cpp sources
OBJS = $(OBJDIR)/y.tab.o  \
       $(OBJDIR)/lex.yy.o \
       $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(filter %.cpp, $(SRCS) ) ) ) \
       $(addprefix $(OBJDIR)/, $(patsubst %.c,   %.o, $(filter %.c,   $(SRCS) ) ) )

# Define the tools we are going to use
CC   = g++
LD   = g++
LEX  = flex
YACC = bison
MV   = mv

# Set up the necessary flags for the tools

# We want debugging and most warnings, but lex/yacc generate some
# static symbols we don't use, so turn off unused warnings to avoid clutter
# Also STL has some signed/unsigned comparisons we want to suppress
#
# DEBUG  = -g
CFLAGS = $(DEBUG) -O3 -Wall -Wno-unused -Wno-sign-compare -I. -I$(SHSRCDIR)

# The -d flag tells lex to set up for debugging. Can turn on/off by
# setting value of global yy_flex_debug inside the scanner itself
LEXFLAGS = -d

# The -d flag tells yacc to generate header with token types
# The -v flag writes out a verbose description of the states and conflicts
# The -t flag turns on debugging capability
# The -y flag means imitate yacc's output file naming conventions
YACCFLAGS = -dvty

# Link with standard c library, math library, and lex library
LIBS = -lc -lm -ll

$(BIN): $(OBJS) $(SHOBJS)
	$(LD) $(OBJS) $(SHOBJS) $(LIBS) -o $@

$(OBJDIR)/lex.yy.c: scanner.l parser.y $(OBJDIR)/y.tab.c 
	$(LEX) $(LEXFLAGS) -o$@ scanner.l

$(OBJDIR)/y.tab.c: parser.y
	$(YACC) $(YACCFLAGS) parser.y
	$(MV) y.output $(OBJDIR)
	$(MV) y.tab.c  $(OBJDIR)
	$(MV) y.tab.h  $(OBJDIR)


$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(SHOBJDIR)/%.o: $(SHSRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*
	rm -f $(SHOBJDIR)/*

Type.o: Type.cpp Type.hpp
Path.o: Path.cpp Path.hpp utility.hpp Node.hpp Type.hpp ModelPrefix.hpp Transistor.hpp
Node.o: Node.cpp Node.hpp Type.hpp utility.hpp Path.hpp
Gnd.o: Gnd.cpp Gnd.hpp Node.hpp Type.hpp utility.hpp
Vdd.o: Vdd.cpp Vdd.hpp Node.hpp Type.hpp utility.hpp
Capacitor.o: Capacitor.cpp Capacitor.hpp
Internal.o: Internal.cpp Internal.hpp Node.hpp Type.hpp utility.hpp ModelPrefix.hpp
Transistor.o: Transistor.cpp Transistor.hpp Node.hpp Type.hpp utility.hpp Vdd.hpp Gnd.hpp
Subcircuit.o: Subcircuit.cpp Subcircuit.hpp Node.hpp Type.hpp utility.hpp Transistor.hpp Input.hpp Internal.hpp ModelPrefix.hpp
Input.o: Input.cpp Input.hpp Internal.hpp Node.hpp Type.hpp utility.hpp Subcircuit.hpp Transistor.hpp Vdd.hpp Gnd.hpp Path.hpp ModelPrefix.hpp
CircuitParser.o: CircuitParser.cpp CircuitParser.hpp Subcircuit.hpp Node.hpp Type.hpp utility.hpp Transistor.hpp Input.hpp Internal.hpp
Instance.o: Instance.cpp Instance.hpp Subcircuit.hpp Node.hpp Type.hpp utility.hpp Transistor.hpp Input.hpp Internal.hpp
diogen.o: diogen.cpp utility.hpp scanner.hpp parser.hpp Transistor.hpp Node.hpp Type.hpp Subcircuit.hpp Input.hpp Internal.hpp y.tab.hpp CircuitParser.hpp


