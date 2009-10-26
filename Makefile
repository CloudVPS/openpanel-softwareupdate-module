include makeinclude

OBJ	= main.o version.o

all: module.xml softwareupdatemodule.exe down_swupd.png
	grace mkapp softwareupdatemodule

down_swupd.png: swupd.png
	convert -modulate 50,100,100 swupd.png down_swupd.png

module.xml: module.def
	mkmodulexml < module.def > module.xml

version.cpp:
	grace mkversion version.cpp

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
