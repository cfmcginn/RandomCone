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

all: mkdirBin mkdirPdf mkdirOutput bin/quickCMSTableGen.exe bin/processForestForRC.exe bin/processForestForImb.exe bin/makeHistRC.exe bin/plotHistRC.exe bin/makeHistImb.exe bin/plotHistImb.exe bin/smearPPToCent.exe

mkdirBin:
	$(MKDIR_BIN)

mkdirOutput:
	$(MKDIR_OUTPUT)

mkdirPdf:
	$(MKDIR_PDF)


bin/processForestForRC.exe: src/processForestForRC.C
	$(CXX) $(CXXFLAGS) src/processForestForRC.C $(ROOT) $(INCLUDE) -o bin/processForestForRC.exe 

bin/processForestForImb.exe: src/processForestForImb.C
	$(CXX) $(CXXFLAGS) src/processForestForImb.C $(ROOT) $(INCLUDE) -o bin/processForestForImb.exe 

bin/makeHistRC.exe: src/makeHistRC.C
	$(CXX) $(CXXFLAGS) src/makeHistRC.C $(ROOT) $(INCLUDE) -o bin/makeHistRC.exe

bin/makeHistImb.exe: src/makeHistImb.C
	$(CXX) $(CXXFLAGS) src/makeHistImb.C $(ROOT) $(INCLUDE) -o bin/makeHistImb.exe

bin/plotHistRC.exe: src/plotHistRC.C
	$(CXX) $(CXXFLAGS) src/plotHistRC.C $(ROOT) $(INCLUDE) -o bin/plotHistRC.exe

bin/quickCMSTableGen.exe: src/quickCMSTableGen.C
	$(CXX) $(CXXFLAGS) src/quickCMSTableGen.C $(ROOT) $(INCLUDE) -o bin/quickCMSTableGen.exe

bin/plotHistImb.exe: src/plotHistImb.C
	$(CXX) $(CXXFLAGS) src/plotHistImb.C $(ROOT) $(INCLUDE) -o bin/plotHistImb.exe

bin/smearPPToCent.exe: src/smearPPToCent.C
	$(CXX) $(CXXFLAGS) src/smearPPToCent.C $(ROOT) $(INCLUDE) -o bin/smearPPToCent.exe


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
