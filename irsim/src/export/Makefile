# Buildfile for export
CC       = g++
BINDIR   = ../../bin
OBJDIR   = ../../obj
SRCS     = export.c

BIN  = $(BINDIR)/export

OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(filter %.c, $(SRCS) ) ) )

CFLAGS = -O3

all: dirs $(BIN)

default: dirs $(OBJS)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)


$(OBJDIR)/%.o: %.c dirs
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $(BIN)

clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*
