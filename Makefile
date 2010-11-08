# This file is part of OpenPanel - The Open Source Control Panel
# OpenPanel is free software: you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation, using version 3 of the License.
#
# Please note that use of the OpenPanel trademark may be subject to additional 
# restrictions. For more information, please visit the Legal Information 
# section of the OpenPanel website on http://www.openpanel.com/


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
	/usr/lib/opencore/libcoremodule.a

clean:
	rm -f *.o *.exe
	rm -rf softwareupdatemodule.app
	rm -f softwareupdatemodule

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../opencore/api/c++/include -c $<
