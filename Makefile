IDIR =./include
CC=g++
CFLAGS=-O3 -w -I$(IDIR)

ODIR=./obj
LIBS=-lm -lpthread
SRC = ./src

_DEPS = functional_cache.h cache_hierarchy.h knobs.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = functional_cache.o cache_hierarchy.o driver.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRC)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

driver: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 