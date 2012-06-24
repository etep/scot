# makefile for the make command
BINDIR    = ../bin
OBJDIR    = ../obj
RANDDIR	  = ../stocc
SHAREDDIR = ../../shared
GGPDIR    = .
BIN       = $(BINDIR)/ciropt
CC        = g++ -Wno-deprecated

SHAREDOBJDIR = $(SHAREDDIR)/obj
SHAREDSRCDIR = $(SHAREDDIR)/src

RANDOBJS  = stoc1.o          \
            mersenne.o       \
            userintf.o

SHAREDOBJS = cnvt.o \
             gp.o

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
             symtab.o         \
             $(RANDOBJS)




LIBS      =
INCLUDES  = -I$(SHAREDSRCDIR) -I$(GGPDIR) -I$(RANDDIR)
CFLAGS    = -O3 $(DEBUGFLAG)
LEX1      = flex -Pciropt
YACC1     = bison -d -p ciropt
LEX2      = flex -Pdio
YACC2     = bison -d -p dio

# -- old -- CLEANFILES  = *.o lex.*.c *.tab.cpp *.tab.hpp *.out *.m

OBJS  = $(addprefix $(OBJDIR)/, $(OBJECTS) )
SOBJS = $(addprefix $(SHAREDOBJDIR)/, $(SHAREDOBJS) )

default: dirs $(BIN)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(SHAREDOBJDIR)

$(BINDIR)/ciropt: $(OBJS) $(SOBJS)
	$(CC) $(OBJS) $(SOBJS) $(LIBS) -o $@
   
$(OBJDIR)/ciropt.o: ciropt.cpp opt.hpp network.hpp $(GGPDIR)/symtab.hpp $(SHAREDSRCDIR)/gp.hpp $(SHAREDSRCDIR)/cnvt.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ ciropt.cpp

$(OBJDIR)/opt.o: opt.cpp opt.hpp analyzer.hpp probdist.hpp network.hpp $(SHAREDSRCDIR)/cnvt.hpp $(GGPDIR)/symtab.hpp $(SHAREDSRCDIR)/gp.hpp $(RANDDIR)/randomc.hpp $(RANDDIR)/stocc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ opt.cpp

$(OBJDIR)/analyzer.o: analyzer.cpp analyzer.hpp opt.hpp network.hpp probdist.hpp $(SHAREDSRCDIR)/cnvt.hpp $(GGPDIR)/symtab.hpp $(SHAREDSRCDIR)/gp.hpp $(RANDDIR)/stocc.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ analyzer.cpp

$(OBJDIR)/network.o: network.cpp network.hpp opt.hpp analyzer.hpp $(SHAREDSRCDIR)/cnvt.hpp $(GGPDIR)/symtab.hpp $(SHAREDSRCDIR)/gp.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ network.cpp


$(OBJDIR)/symtab.o: $(GGPDIR)/symtab.cpp $(GGPDIR)/symtab.hpp $(SHAREDSRCDIR)/cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(GGPDIR)/symtab.cpp

$(SHAREDOBJDIR)/gp.o: $(SHAREDSRCDIR)/gp.cpp $(SHAREDSRCDIR)/gp.hpp $(GGPDIR)/ggp.hpp $(SHAREDSRCDIR)/cnvt.hpp $(GGPDIR)/symtab.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/lex.ciropt.o: $(OBJDIR)/lex.ciropt.c $(OBJDIR)/ciropt.tab.hpp definerandomch
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(OBJDIR)/lex.ciropt.c

$(OBJDIR)/lex.ciropt.c: ciropt.l
	$(LEX1) ciropt.l
	cat definerandomch lex.ciropt.c > $(OBJDIR)/lex.ciropt.c
	rm lex.ciropt.c

$(OBJDIR)/ciropt.tab.o: $(OBJDIR)/ciropt.tab.cpp network.hpp opt.hpp analyzer.hpp $(GGPDIR)/symtab.hpp $(SHAREDSRCDIR)/gp.hpp $(SHAREDSRCDIR)/cnvt.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(OBJDIR)/ciropt.tab.cpp

$(OBJDIR)/ciropt.tab.cpp: $(OBJDIR)/ciropt.tab.hpp
	

$(OBJDIR)/ciropt.tab.hpp: ciropt.ypp
	$(YACC1) ciropt.ypp
	mv ciropt.tab.cpp $(OBJDIR)
	mv ciropt.tab.hpp $(OBJDIR)

$(OBJDIR)/lex.dio.o: $(OBJDIR)/lex.dio.c $(OBJDIR)/dio.tab.hpp $(SHAREDSRCDIR)/gp.hpp $(GGPDIR)/symtab.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(OBJDIR)/lex.dio.c

$(OBJDIR)/lex.dio.c: dio.l
	$(LEX2) dio.l
	mv lex.dio.c $(OBJDIR)

$(OBJDIR)/dio.tab.o: $(OBJDIR)/dio.tab.cpp network.hpp $(SHAREDSRCDIR)/gp.hpp $(GGPDIR)/symtab.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(OBJDIR)/dio.tab.cpp

$(OBJDIR)/dio.tab.cpp: $(OBJDIR)/dio.tab.hpp
	

$(OBJDIR)/dio.tab.hpp: dio.ypp
	$(YACC2) dio.ypp
	mv dio.tab.cpp $(OBJDIR)
	mv dio.tab.hpp $(OBJDIR)

$(OBJDIR)/probdist.o: probdist.hpp probdist.cpp mat.hpp $(SHAREDSRCDIR)/cnvt.hpp $(RANDDIR)/stocc.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ probdist.cpp

$(OBJDIR)/mat.o: mat.hpp mat.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ mat.cpp

$(SHAREDOBJDIR)/cnvt.o: $(SHAREDSRCDIR)/cnvt.cpp $(SHAREDSRCDIR)/cnvt.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/stoc1.o: $(RANDDIR)/stoc1.cpp $(RANDDIR)/stocc.hpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(RANDDIR)/stoc1.cpp

$(OBJDIR)/mersenne.o: $(RANDDIR)/mersenne.cpp $(RANDDIR)/randomc.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(RANDDIR)/mersenne.cpp

$(OBJDIR)/userintf.o: $(RANDDIR)/userintf.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $(RANDDIR)/userintf.cpp

$(OBJDIR)/tcode: tcode.cpp
	$(CC) -o $@ tcode.cpp

clean:
	rm -f $(OBJDIR)/*
	rm -f $(SHAREDOBJDIR)/*
	rm -f $(BINDIR)/*

backup: clean
	tar cvfX bu/lbu.tar ex .

leland: clean
	cp -iup * ~/leland/research/ciropt/



