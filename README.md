# RandomCone
For use with arbitrary ROOT Ntuples of basic datatypes, array of basic datatypes, and vector of basic datatypes

To build run

make

Try following workflow

./bin/processForestForRC.exe <inFileName> <paramFileName>

with paramFileName=configs/simpleConfig.txt and inFileName your input. Should output to output/<DATE>

then do

./bin/makeHistRC.exe <inFileName>

with inFileName=output/<DATE>/<FILE MADE IN LAST STEP>. this will again output to output/<DATE>

then do

./bin/plotHistRC.exe <inFileName>

with inFileName=output/<DATE>/<FILE MADE IN LAST STEP>. this will output to pdfDir/<DATE> some plots
