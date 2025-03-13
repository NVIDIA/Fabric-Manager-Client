IDIR := ∕usr∕include
CXXFLAGS = -I $(IDIR) $(shell pkg-config --cflags jsoncpp)

LDIR := ∕usr∕lib
LDFLAGS= -L$(LDIR) -lnvfm -ljsoncpp

fmpm: fmpm.o
	g++ -o $@ $< $(CXXFLAGS) $(LDFLAGS)

clean:
	-@rm -f fmpm.o
	-@rm -f fmpm
