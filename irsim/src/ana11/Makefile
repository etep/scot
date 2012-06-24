TARGETS   = $(OBJDIR)/ana.o $(BINDIR)/anXhelper
CC        = g++
CFLAGS    = -O3
BINDIR    = ../../bin
OBJDIR    = ../../obj
INCLUDES  =	-I../include
LIBS      =
EXPORTBIN = $(BINDIR)/export
EXPORTED  = ana_glob.h

SRCS = analyzer.c     \
       base.c         \
       convert.c      \
       coords.c       \
       deltaT.c       \
       event.c        \
       defaults.c     \
       graphics.c     \
       icon.c         \
       menu.c         \
       movetot.c      \
       movetrace.c    \
       namelen.c      \
       postscript.c   \
       setsteps.c     \
       scrollbar.c    \
       textwind.c     \
       update.c       \
       window.c       \
       wm.c           \
       zoom.c

OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(filter %.c, $(SRCS) ) ) )

all: dirs $(TARGETS)

default: dirs $(TARGETS)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)

$(OBJDIR)/ana.o: $(EXPORTED) $(SRCS) $(OBJS) anXhelper.c
	ld -r $(OBJS) $(LIBS) -o $(OBJDIR)/ana.o

$(BINDIR)/anXhelper: anXhelper.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BINDIR)/anXhelper anXhelper.c

$(OBJDIR)/%.o: %.c dirs
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(EXPORTED): $(SRCS) $(EXPORTBIN)
	@echo making $(EXPORTED)
	@$(EXPORTBIN) $(SRCS) $(EXPORTED)

clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*
	rm -f ana_glob.h
