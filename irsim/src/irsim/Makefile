CC     = g++
DEBUG  = -g
CFLAGS = -O3

BINDIR   = ../../bin
OBJDIR   = ../../obj
INCLUDES =	-I../include -I$(OBJDIR)
BINNAME  = irsim
EXPORT   = $(BINDIR)/export
BIN      = $(BINDIR)/$(BINNAME)

HDRS =	../include/net.h

ANA     = $(OBJDIR)/ana.o
EXPORTS = globals.h

# for no analyzer (systems without X) define X = NO and XLIB = <empty>
#X	= NO
#XLIB	= 
X      = 11
XLIB   = -lX11
LIBS   = $(XLIB) -lm
LFLAGS = -L/usr/X11/lib

# for no incremental simulation (and fault simulation) define incsim = incstub
incsim = incsim
#incsim = incstub

SRCS =  access.c     \
        binsim.c     \
        cad_dir.c    \
        config.c     \
        conn_list.c  \
        eval.c       \
        fio.c        \
        hist.c       \
        hist_io.c    \
        intr.c       \
        $(incsim).c  \
        mem.c        \
        netupdate.c  \
        network.c    \
        newrstep.c   \
        nsubrs.c     \
        parallel.c   \
        prints.c     \
        rsim.c       \
        sched.c      \
        sim.c        \
        sstep.c      \
        stack.c      \
        tpos.c       \
        usage.c      \
        version.c    \
        subckt.c

OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(filter %.c, $(SRCS) ) ) )

GENSPK = $(BINDIR)/genspktbl
GENTBL = $(BINDIR)/gentbl

all: dirs $(BIN)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)

$(BIN): user_stub.c $(OBJDIR)/libIrsim.a
	$(CC) -o $(BIN) $(LFLAGS) $(OBJDIR)/libIrsim.a user_stub.c $(LIBS)

$(OBJDIR)/libIrsim.a: $(OBJDIR)/spiketbl.c $(OBJDIR)/stables.c $(OBJS) $(ANA)
	ar rv  $(OBJDIR)/libIrsim.a $(OBJS) $(ANA)
	ranlib $(OBJDIR)/libIrsim.a

clean:
	rm -f $(BINDIR)/*
	rm -f $(OBJDIR)/*
	rm -f globals.h
	rm -f ../anaNO/ana_glob.h
	rm -f ../ana11/ana_glob.h

$(EXPORT): ../export/export.c ../export/Makefile
	( cd ../export ; make )

$(EXPORTS): $(SRCS) $(EXPORT) ../ana11/analyzer.c ../anaNO/ana.c
	@echo making $(EXPORTS)
	@$(EXPORT) $(SRCS) ../anaNO/ana.c $(EXPORTS)

$(OBJDIR)/stables.c: $(GENTBL)
	$(GENTBL)
	mv stables.c $(OBJDIR)

$(GENTBL): gentbl.c
	$(CC) $(CFLAGS) $(INCLUDES) gentbl.c -o $(GENTBL)

$(GENSPK): genspktbl.c
	$(CC) $(CFLAGS) $(INCLUDES) genspktbl.c -lm -o $(GENSPK)

$(OBJDIR)/spiketbl.c: $(GENSPK)
	$(GENSPK) spiketbl.c
	mv spiketbl.c $(OBJDIR)

$(OBJDIR)/%.o: %.c $(GENTBL) $(GENSPK) $(EXPORTS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(ANA): $(EXPORT)
	( cd ../ana$(X) ; make )
