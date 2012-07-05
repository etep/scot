BINDIR    = ../bin
OBJDIR    = ../obj
RANDDIR	  = ../stocc
SHROOT    = ../../shared
BIN       = $(BINDIR)/ciropt
CC        = g++ -Wno-deprecated

SHOBJDIR = $(SHROOT)/obj
SHSRCDIR = $(SHROOT)/src

RANDOBJS   = stoc1.o          \
             mersenne.o       \
             userintf.o

SHOBJECTS  = symtab.o         \
             cnvt.o           \
             gp.o             \
			 SystemWrapper.o

OBJECTS    = ciropt.o         \
             opt.o            \
             analyzer.o       \
             network.o        \
             probdist.o       \
             mat.o            \
             lex.ciropt.o     \
             ciropt.tab.o     \
             lex.dio.o        \
             dio.tab.o        \
             $(RANDOBJS)

LIBS      =
INCLUDES  = -I. -I$(SHSRCDIR) -I$(RANDDIR)
CFLAGS    = -O3 $(DEBUGFLAG)
LEX1      = flex -Pciropt
YACC1     = bison -d -p ciropt
LEX2      = flex -Pdio
YACC2     = bison -d -p dio

OBJS   = $(addprefix $(OBJDIR)/, $(OBJECTS) )
SHOBJS = $(addprefix $(SHOBJDIR)/, $(SHOBJECTS) )

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


$(BINDIR)/ciropt: $(OBJS) $(SHOBJS)
	$(CC) $(OBJS) $(SHOBJS) $(LIBS) -o $@

$(OBJDIR)/lex.ciropt.c: ciropt.l
	$(LEX1) ciropt.l
	cat definerandomch lex.ciropt.c > $(OBJDIR)/lex.ciropt.c
	rm lex.ciropt.c

$(OBJDIR)/lex.dio.c: dio.l
	$(LEX2) dio.l
	mv lex.dio.c $(OBJDIR)

$(OBJDIR)/ciropt.tab.hpp: ciropt.ypp
	$(YACC1) ciropt.ypp
	mv ciropt.tab.cpp $(OBJDIR)
	mv ciropt.tab.hpp $(OBJDIR)

$(OBJDIR)/dio.tab.hpp: dio.ypp
	$(YACC2) dio.ypp
	mv dio.tab.cpp $(OBJDIR)
	mv dio.tab.hpp $(OBJDIR)
 
$(OBJDIR)/ciropt.o:   ciropt.cpp       opt.hpp network.hpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/gp.hpp $(SHSRCDIR)/cnvt.hpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/opt.o:      opt.cpp          opt.hpp  analyzer.hpp probdist.hpp network.hpp $(SHSRCDIR)/cnvt.hpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/gp.hpp $(RANDDIR)/randomc.hpp $(RANDDIR)/stocc.hpp
$(OBJDIR)/analyzer.o: analyzer.cpp     analyzer.hpp opt.hpp network.hpp probdist.hpp $(SHSRCDIR)/cnvt.hpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/gp.hpp $(RANDDIR)/stocc.hpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/network.o:  network.cpp      network.hpp opt.hpp analyzer.hpp $(SHSRCDIR)/cnvt.hpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/gp.hpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/probdist.o: probdist.hpp     probdist.cpp mat.hpp $(SHSRCDIR)/cnvt.hpp $(RANDDIR)/stocc.hpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/mat.o:      mat.cpp          mat.hpp

$(OBJDIR)/lex.ciropt.o:   $(OBJDIR)/lex.ciropt.c   $(OBJDIR)/ciropt.tab.hpp definerandomch
$(OBJDIR)/ciropt.tab.o:   $(OBJDIR)/ciropt.tab.cpp network.hpp opt.hpp analyzer.hpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/gp.hpp $(SHSRCDIR)/cnvt.hpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/lex.dio.o:      $(OBJDIR)/lex.dio.c      $(OBJDIR)/dio.tab.hpp $(SHSRCDIR)/gp.hpp $(SHSRCDIR)/symtab.hpp
$(OBJDIR)/ciropt.tab.cpp: $(OBJDIR)/ciropt.tab.hpp
$(OBJDIR)/dio.tab.cpp:    $(OBJDIR)/dio.tab.hpp

$(OBJDIR)/stoc1.o:    $(RANDDIR)/stoc1.cpp    $(RANDDIR)/stocc.hpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/mersenne.o: $(RANDDIR)/mersenne.cpp $(RANDDIR)/randomc.hpp
$(OBJDIR)/userintf.o: $(RANDDIR)/userintf.cpp


$(SHOBJDIR)/cnvt.o:   $(SHSRCDIR)/cnvt.cpp   $(SHSRCDIR)/cnvt.hpp
$(SHOBJDIR)/symtab.o: $(SHSRCDIR)/symtab.cpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/cnvt.hpp
$(SHOBJDIR)/gp.o:     $(SHSRCDIR)/gp.cpp     $(SHSRCDIR)/gp.hpp     $(SHSRCDIR)/cnvt.hpp $(SHSRCDIR)/symtab.hpp $(SHSRCDIR)/ggp.hpp

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(SHOBJDIR)


clean:
	rm -f $(OBJDIR)/*
	rm -f $(SHOBJDIR)/*
	rm -f $(BINDIR)/*


