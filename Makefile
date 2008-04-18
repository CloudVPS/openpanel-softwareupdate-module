include makeinclude

OBJ	= main.o version.o

all: module.xml softwareupdatemodule.exe
	mkapp softwareupdatemodule

module.xml: module.def
	mkmodulexml < module.def > module.xml

version.cpp:
	mkversion version.cpp

softwareupdatemodule.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o softwareupdatemodule.exe $(OBJ) $(LIBS) \
	../opencore/api/c++/lib/libcoremodule.a

clean:
	rm -f *.o *.exe
	rm -rf softwareupdatemodule.app
	rm -f softwareupdatemodule

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../opencore/api/c++/include -c $<
