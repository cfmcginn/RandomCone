CXX = g++
#O3 for max optimization (go to 0 for debug)
CXXFLAGS = -Wall -Werror -O0 -Wextra -Wno-unused-local-typedefs -Wno-deprecated-declarations -std=c++11 -g
ifeq "$(GCCVERSION)" "1"
  CXXFLAGS += -Wno-error=misleading-indentation
endif

INCLUDE=-I $(PWD)
ROOT=`root-config --cflags --glibs`

MKDIR_BIN=mkdir -p $(PWD)/bin
MKDIR_OUTPUT=mkdir -p $(PWD)/output
MKDIR_PDF=mkdir -p $(PWD)/pdfDir

all: mkdirBin mkdirPdf mkdirOutput bin/quickCMSTableGen.exe bin/processForestForRC.exe bin/makeHistRC.exe bin/plotHistRC.exe

mkdirBin:
	$(MKDIR_BIN)

mkdirOutput:
	$(MKDIR_OUTPUT)

mkdirPdf:
	$(MKDIR_PDF)


bin/processForestForRC.exe: src/processForestForRC.C
	$(CXX) $(CXXFLAGS) src/processForestForRC.C $(ROOT) $(INCLUDE) -o bin/processForestForRC.exe

bin/makeHistRC.exe: src/makeHistRC.C
	$(CXX) $(CXXFLAGS) src/makeHistRC.C $(ROOT) $(INCLUDE) -o bin/makeHistRC.exe

bin/plotHistRC.exe: src/plotHistRC.C
	$(CXX) $(CXXFLAGS) src/plotHistRC.C $(ROOT) $(INCLUDE) -o bin/plotHistRC.exe

bin/quickCMSTableGen.exe: src/quickCMSTableGen.C
	$(CXX) $(CXXFLAGS) src/quickCMSTableGen.C $(ROOT) $(INCLUDE) -o bin/quickCMSTableGen.exe

clean:
	rm -f ./*~
	rm -f ./#*#
	rm -f bash/*~
	rm -f bash/#*#
	rm -f bin/*.exe
	rm -rf bin
	rm -f configs/*~
	rm -f configs/#*#
	rm -f include/*~
	rm -f include/#*#
	rm -f input/*~
	rm -f input/#*#
	rm -f src/*~
	rm -f src/#*#
