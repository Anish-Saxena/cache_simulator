IDIR =./include
CC=g++
CFLAGS=-std=c++11 -w -O3 -march=native -I$(IDIR)

ODIR=./obj
LIBS=-lm -lpthread
SRC = ./src

_DEPS = functional_cache.h cache_hierarchy.h knobs.h miss_categorisation.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = functional_cache.o cache_hierarchy.o driver.o miss_categorisation.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRC)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

driver: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
