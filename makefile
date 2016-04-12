GNUREC=-O3 -ffast-math -funroll-all-loops -fpeel-loops -ftracer -funswitch-loops -funit-at-a-time -pthread
GO=$(GNUREC)
CXX=g++ $(GO) 

INCLUDES=-I./tclap-1.2.1/include/ 
CXXFLAGS=-c $(INCLUDES)
LDFLAGS=$(LIBS) 
INLIBS=-lgcc_s -lpthread -lc -lm

alignSrc=mmAligner.cpp mmEM.cpp
SOURCES=$(alignSrc)
alignObj=$(alignSrc:.cpp=.o)
OBJECTS=$(alignObj)

EXECUTABLE=m2m-aligner

all: $($SOURCES) $(EXECUTABLE)

$(EXECUTABLE):	$(OBJECTS) 
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@ $(INLIBS)

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

clean:	
	rm -f $(EXECUTABLE) $(OBJECTS)
