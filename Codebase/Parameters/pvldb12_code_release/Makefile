CC=g++
CFLAGS=-c -O3 
DEBFLAGS=-c -g
LEADERSPATH=../leaders-3.0
OFLAGS=-DPARSEINPUT_H=1

all: InfluenceModels

InfluenceModels: common.o Edge.o anyoption.o InputParserForPCModel.o PCModel.o InputParserForTraining.o Training.o MC.o CoverageTest.o InfluenceModels.o 
	$(CC) -O3 common.o Edge.o anyoption.o InputParserForPCModel.o PCModel.o InputParserForTraining.o Training.o MC.o CoverageTest.o InfluenceModels.o -o InfluenceModels

common.o: common.cc
	$(CC) $(CFLAGS) common.cc	

Edge.o: Edge.cc
	$(CC) $(CFLAGS) Edge.cc	

anyoption.o: anyoption.cc
	$(CC) $(CFLAGS) anyoption.cc 

InputParserForPCModel.o: InputParserForPCModel.cc
	$(CC) $(CFLAGS) InputParserForPCModel.cc

PCModel.o: PCModel.cc
	$(CC) $(CFLAGS) PCModel.cc

InputParserForTraining.o: InputParserForTraining.cc
	$(CC) $(CFLAGS) InputParserForTraining.cc

Training.o: Training.cc
	$(CC) $(CFLAGS) Training.cc

MC.o: MC.cc
	$(CC) $(CFLAGS) MC.cc

CoverageTest.o: CoverageTest.cc
	$(CC) $(CFLAGS) CoverageTest.cc

InfluenceModels.o: InfluenceModels.cc
	$(CC) $(CFLAGS) InfluenceModels.cc

debug: common_debug.o Edge_debug.o anyoption_debug.o InputParserForPCModel_debug.o PCModel_debug.o InputParserForTraining_debug.o Training_debug.o MC_debug.o CoverageTest_debug.o InfluenceModels_debug.o
	$(CC) -g common.o Edge.o anyoption.o InputParserForPCModel.o PCModel.o InputParserForTraining.o Training.o MC.o CoverageTest.o InfluenceModels.o -o InfluenceModels_debug

common_debug.o: common.cc
	$(CC) $(DEBFLAGS) common.cc

Edge_debug.o: Edge.cc
	$(CC) $(DEBFLAGS) Edge.cc

anyoption_debug.o: anyoption.cc
	$(CC) $(DEBFLAGS) anyoption.cc 

InputParserForPCModel_debug.o: InputParserForPCModel.cc
	$(CC) $(DEBFLAGS) InputParserForPCModel.cc

PCModel_debug.o: PCModel.cc
	$(CC) $(DEBFLAGS) PCModel.cc

InputParserForTraining_debug.o: InputParserForTraining.cc
	$(CC) $(DEBFLAGS) InputParserForTraining.cc

Training_debug.o: Training.cc
	$(CC) $(DEBFLAGS) Training.cc

MC_debug.o: MC.cc
	$(CC) $(DEBFLAGS) MC.cc

CoverageTest_debug.o: CoverageTest.cc
	$(CC) $(DEBFLAGS) CoverageTest.cc

InfluenceModels_debug.o: InfluenceModels.cc
	$(CC) $(DEBFLAGS) InfluenceModels.cc

clean:
	rm -f *.o *~

