#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
# Modified by Vasco Baptista to remove unecessary entries and clean the file   #
#------------------------------------------------------------------------------#

CXX = g++
LD = g++

INC = 
CFLAGS = -Wextra -Wall -fexceptions -O2
RESINC = 
LIBDIR = 
LIB = -lwiringPi
LDFLAGS = -s
OBJDIR = obj
DEP = 
OUT_BIN = bin/daylapse

OBJ = $(OBJDIR)/main.o $(OBJDIR)/snapframes.o $(OBJDIR)/sunriset.o

all: release

clean: clean_release

before_release: 
	test -d bin || mkdir -p bin
	test -d $(OBJDIR) || mkdir -p $(OBJDIR)

release: before_release out_release

out_release: before_release $(OBJ) $(DEP)
	$(LD) $(LIBDIR) -o $(OUT_BIN) $(OBJ) $(LDFLAGS) $(LIB)

$(OBJDIR)/main.o: main.cpp
	$(CXX) $(CFLAGS) $(INC) -c main.cpp -o $(OBJDIR)/main.o

$(OBJDIR)/snapframes.o: snapframes.cpp
	$(CXX) $(CFLAGS) $(INC) -c snapframes.cpp -o $(OBJDIR)/snapframes.o

$(OBJDIR)/sunriset.o: sunriset.cpp
	$(CXX) $(CFLAGS) $(INC) -c sunriset.cpp -o $(OBJDIR)/sunriset.o

clean_release: 
	rm -f $(OBJ) $(OUT_BIN)
	rm -rf bin
	rm -rf $(OBJDIR)

.PHONY: before_release clean_release

