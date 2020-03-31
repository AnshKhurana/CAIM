#ifndef COVERAGETEST_H
#define COVERAGETEST_H

#include "common.h"
#include "InputParserForPCModel.h"
#include <vector>
#include <ctime>

#include "PCModel.h"
#include "MC.h"

//using namespace PartialCredits;
using namespace _MC;

namespace PartialCredits {

class CoverageTest {

	// parameters from config files	
	int pi;
	string outdir;
	string m;
	PropModels model;

	int maxTuples;
	
	AnyOption* opt; 

	// other parameters	
	// influence vector	
	// head is the tuple having max ts
	PartialCredits::ActionLogTuple *head, *tail;	// universal head and tail of the queue
	
	// parameters pointing to data structs and other classes
	HashTreeCube* AM;
	InputParserForPCModel* input;
	
	unsigned int maxWindowSize;
	PartialCredits::UsersTuplesMap* tuplesInCurrentWindow;
	long actionsDone;
	const char* actionsFile;
	
	// flickr
	unsigned long long countLines;


	UserList actions_in_testing;
	ActionCov covs;

	time_t startTime;

	map<UID, int> u_counts;
	map<AID, UserList> SeedSets;
	map<AID, map<UID, float>* > NC;
	
public:
	CoverageTest (AnyOption* opt);
	~CoverageTest ();
	void doAll();
	void scanActionsLog(); // former moveTimeWindow
	unsigned int getMaxWindowSize() {
		return maxWindowSize;
	}
	
	//void writeOutput(string outputfile);
	void writeOutput(unsigned long, ActionCov* covs = NULL);
	void writeCountsInFileNew();
	void readActionsInTest();

private:
	void propagate(PartialCredits::ActionLogTuple* tuple); // former insert function
	void computeMCCov(MC* mc);
	void resetActionQueue();
	void openOutputFiles();
	void closeOutputFiles();
	float getTime() const;
	void setModel();
	void readSeedFile();
};
}
#endif
