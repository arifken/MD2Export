MAYA_DIR	= /Applications/Autodesk/maya2010
MAYA_LOCATION	= $(MAYA_DIR)/Maya.app/Contents

include $(MAYA_DIR)/devkit/plug-ins/buildconfig

TARGET		= MD2Export
OBJECTS		= MD2Exporter.o MD2Object.o MD2Writer.o
LFLAGS		= \
	-fno-gnu-keywords -fpascal-strings \
	-isysroot /Developer/SDKs/MacOSX10.6.sdk \
	-arch i386 -headerpad_max_install_names \
	-bundle

.SUFFIXES: .cpp .o

.cpp.o:
	$(C++) -c $(INCLUDES) $(C++FLAGS) $<

all: clean $(TARGET).$(EXT)

$(TARGET).$(EXT): $(OBJECTS) 
	$(LD) -o $@ $(OBJECTS) $(LIBS) -lOpenMayaAnim

install:
	install -m 644 $(TARGET).$(EXT) $(MAYA_LOCATION)/MacOS/plug-ins/
	install -m 644 MD2ExportOptionsScript.mel $(HOME)/Documents/maya/scripts/

clean:
	rm -f $(TARGET).$(EXT) *.o
