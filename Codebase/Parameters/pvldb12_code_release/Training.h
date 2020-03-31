#ifndef TRAINING_H
#define TRAINING_H

#include "common.h"
#include "InputParserForTraining.h"
#include <vector>
#include <ctime>


namespace Train {

typedef map<UID, ActionLogTuple*> UsersTuplesMap;
	
class Training {

	// parameters from config files	
	int pi;
	string outdir;
	string m;
	int  maxTuples;
	int dynamicFG;
	
	// parameters constructed directly from config file parameters
	Models model;
	int modelType;
	

	// other parameters	
	// influence vector	
	// head is the tuple having max ts
	ActionLogTuple *head, *tail;	// universal head and tail of the queue
	
	// parameters pointing to data structs and other classes
	HashTreeCube* AM;
	InputParserForTraining* input;
	
	unsigned int maxWindowSize;
	UsersTuplesMap* tuplesInCurrentWindow;
	long actionsDone;
	const char* actionsFile;
	
	// flickr
	int dataset_size;
	string usersCountsFileName;
	string edgesCountsFileName;
	ofstream usersCountsFile;
	ofstream edgesCountsFile;
	unsigned long long countLines;

	UsersCounts u_counts; // maintain count of actions each user performs in training set
	UsersCounts countParents; // maintain the number of actions for each user for which she got influenced by someone else
	int computeUserInf;

	const char* trainingActionsFile;
	set<AID> actions_in_training;

	// DB connection
	//DBConnect* dbc;
	string GraphTableName;

	time_t startTime;
	
public:
	Training (AnyOption* opt);
	~Training ();
	void doAll();
	void scanActionsLog(); // former moveTimeWindow
	void printQ(unsigned int actionId) ;
	unsigned int getMaxWindowSize() {
		return maxWindowSize;
	}
	
	void writeCountsInFile();
	void writeCountsInFileNew();
	void readActionsInTraining();

private:
	void propagate(ActionLogTuple* tuple); // former insert function
	void propagateType4Models(ActionLogTuple* tuple); // former insert function
	unsigned long update(TS tlow); // former removeOldestNodes
	void resetActionQueue();
	void openOutputFiles();
	void closeOutputFiles();
	inline void writeICinFile(UID uid, AID aid, UserList* followers);
	inline vector<UID>* getNeighbors(UID u) const;
	void computeUserInfluencability(ActionLogTuple*);
	float getTime() const;

};
}
#endif
