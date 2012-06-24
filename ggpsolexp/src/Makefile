# makefile for the make command
#COMPILER = CC
#COMPILER = g++ -O
BINDIR   = ../bin
OBJDIR   = ../obj
BIN      = $(BINDIR)/ggpsolexp
INCLUDES = -I. -I$(OBJDIR)
CC      = g++
CFLAGS  = -c -O3 -Wno-deprecated
OBJECTS = ggpsolexp.o gp.o ggp.o lex.ggp.o ggp.tab.o symtab.o d2p.o cnvt.o
LEX     = flex -Pggp
YACC    = bison -d -p ggp
CLEANFILES = lex.ggp.c ggp.tab.cpp ggp.tab.hpp

OBJS = $(addprefix $(OBJDIR)/, $(OBJECTS) )

#CLEANFILES = ggpsolexp *.o lex.ggp.c ggp.tab.cpp ggp.tab.hpp

#MOSEKFLAG = -I$(HM)/mosek/2/tools/h
#MOSEKLINK = $(HM)/mosek/2/tools/platform/linux/intel/dll/libmosek.so.2.0
#MOSEKLINK = $(HM)/mosek/2/tools/platform/solaris/intel/dll/libmosek.so.2.0
default: dirs $(BIN)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
   
$(BIN): $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(MOSEKLINK) -o $@
   
$(OBJDIR)/ggpsolexp.o: ggpsolexp.hpp ggpsolexp.cpp gp.hpp ggp.hpp cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ ggpsolexp.cpp

$(OBJDIR)/ggp.o: ggp.cpp ggp.hpp gp.hpp symtab.h cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ ggp.cpp

$(OBJDIR)/gp.o: gp.cpp gp.hpp symtab.h cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gp.cpp

$(OBJDIR)/lex.ggp.o: $(OBJDIR)/lex.ggp.c $(OBJDIR)/ggp.tab.hpp gp.hpp ggp.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(OBJDIR)/lex.ggp.c: ggp.l
	$(LEX) ggp.l
	mv lex.ggp.c $(OBJDIR)

$(OBJDIR)/ggp.tab.o: $(OBJDIR)/ggp.tab.cpp symtab.h gp.hpp ggp.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(OBJDIR)/ggp.tab.hpp: ggp.ypp
	$(YACC) ggp.ypp
	mv ggp.tab.hpp $(OBJDIR)
	mv ggp.tab.cpp $(OBJDIR)

$(OBJDIR)/ggp.tab.cpp: $(OBJDIR)/ggp.tab.hpp

$(OBJDIR)/symtab.o: symtab.cpp symtab.h cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ symtab.cpp

$(OBJDIR)/d2p.o: d2p.cpp d2p.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ d2p.cpp

$(OBJDIR)/cnvt.o:	cnvt.cpp cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ cnvt.cpp

clean:
	rm -f ../obj/*
	rm -f ../bin/*

backup: clean
	tar cvfX bu/lbu.tar ex .
