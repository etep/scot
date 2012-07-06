BINDIR     = ../bin
OBJDIR     = ../obj
SHDIR      = ../../shared
SHSRCDIR   = $(SHDIR)/src
SHOBJDIR   = $(SHDIR)/obj
BIN        = $(BINDIR)/ggpsolexp
INCLUDES   = -I. -I$(OBJDIR) -I$(SHSRCDIR)
CC         = g++
CFLAGS     = -O3 -Wno-deprecated
LEX        = flex -Pggp
YACC       = bison -d -p ggp
CLEANFILES = lex.ggp.c ggp.tab.cpp ggp.tab.hpp
OBJECTS    = ggpsolexp.o \
             lex.ggp.o   \
             ggp.tab.o   \
             d2p.o
SHOBJECTS  = gp.o             \
             ggp.o            \
             symtab.o         \
             cnvt.o           \
             SystemWrapper.o

OBJS   = $(addprefix $(OBJDIR)/, $(OBJECTS) )
SHOBJS = $(addprefix $(SHOBJDIR)/, $(SHOBJECTS) )

#CLEANFILES = ggpsolexp *.o lex.ggp.c ggp.tab.cpp ggp.tab.hpp

default: dirs $(BIN)

$(SHOBJDIR)/%.o: $(SHSRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: $(OBJDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: $(RANDDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: $(OBJDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: $(OBJDIR)/%.dio.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
   
$(BIN): $(OBJS) $(SHOBJS)
	$(CC) $(OBJS) $(SHOBJS) $(LIBS) -o $@
   
$(OBJDIR)/ggpsolexp.o: ggpsolexp.hpp ggpsolexp.cpp $(SHSRCDIR)/gp.hpp $(SHSRCDIR)/ggp.hpp $(SHSRCDIR)/cnvt.hpp
$(OBJDIR)/d2p.o:       d2p.cpp d2p.hpp

$(OBJDIR)/lex.ggp.o: $(OBJDIR)/lex.ggp.c   $(OBJDIR)/ggp.tab.hpp  $(SHSRCDIR)/gp.hpp  $(SHSRCDIR)/ggp.hpp
$(OBJDIR)/ggp.tab.o: $(OBJDIR)/ggp.tab.cpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/gp.hpp  $(SHSRCDIR)/ggp.hpp
$(OBJDIR)/ggp.tab.cpp: $(OBJDIR)/ggp.tab.hpp

$(SHOBJDIR)/ggp.o:    $(SHSRCDIR)/ggp.cpp    $(SHSRCDIR)/gp.hpp  $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/cnvt.hpp $(SHSRCDIR)/ggp.hpp
$(SHOBJDIR)/gp.o:     $(SHSRCDIR)/gp.cpp     $(SHSRCDIR)/gp.hpp  $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/cnvt.hpp
$(SHOBJDIR)/cnvt.o:	  $(SHSRCDIR)/cnvt.cpp   $(SHSRCDIR)/cnvt.hpp
$(SHOBJDIR)/symtab.o: $(SHSRCDIR)/symtab.cpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/cnvt.hpp

$(OBJDIR)/lex.ggp.c: ggp.l
	$(LEX) ggp.l
	mv lex.ggp.c $(OBJDIR)

$(OBJDIR)/ggp.tab.hpp: ggp.ypp
	$(YACC) ggp.ypp
	mv ggp.tab.hpp $(OBJDIR)
	mv ggp.tab.cpp $(OBJDIR)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(SHOBJDIR)

clean:
	rm -f $(BINDIR)/*
	rm -f $(OBJDIR)/*
	rm -f $(SHOBJDIR)/*

