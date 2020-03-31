#ifndef PCMODEL_H
#define PCMODEL_H

#include "common.h"
#include "InputParserForPCModel.h"
#include <vector>
#include <ctime>

#include "CoverageTest.h"


namespace PartialCredits {
class CoverageTest;
		
struct Node {
	// We have two type of things in the Node
	// 1. The DAG(a) which has the node id, pointers to chidren nodes and
	// 1/indegree
	// 2. We save tc_{S,v}(a) and a flag denoting the iteration number when it
	// was last updated
	// 
	UID uid;
//	float dc; // dc = 1/indegree

//	float tc; // tc = tc_{S,uid} (a)

//	float cov; // cov^{G-S}(uid, a)
	
//	short flagTC; // iteration when tc was last updated
//	short flagCov; // iteration when cov was last updated

	map<Node*, float> children; // tc_{uid, node} (a)
	map<Node*, float> ancestors; // tc_{node, uid} (a)

	Node(UID u) : uid(u)
	{}
};

// for PC model project
// for Influence Matrix IM where IM[u,a] = 
typedef map<AID, Node*> ActionNodeMap;
typedef map<UID, ActionNodeMap> InfCube;


struct Qrow {
	UID v;
	int flag;

	Qrow(UID v1, int flag1) : v(v1), flag(flag1) {}
};

typedef multimap<float, Qrow> FloatUserMap;

	
class PCModel {

	InfCube IM;
	AnyOption* opt;

	// parameters from config files	
	string outdir;
	string m;
	int maxTuples;
	float tol;


//	CoverageTest* covTest;
	

	// other parameters	
	// influence vector	
	// head is the tuple having max ts
	ActionLogTuple *head, *tail;	// universal head and tail of the queue
	
	// parameters pointing to data structs and other classes
	HashTreeCube* AM;
	InputParserForPCModel* input;
	
	unsigned int maxWindowSize;
	UsersTuplesMap* tuplesInCurrentWindow;
	long actionsDone;
	const char* actionsFile;

	int budget;
	
	ofstream outFile;

	// total credit for current action on user UID
	// Used for estimating coverage given a seed set
	// tc_{S,y}(b)
	map<UID, map<AID, float>* > TCb; 
	map<AID, map<UID, float>* >* NC;
	map<AID, UserList>* SeedSets;
	ActionCov* covs;

	UserList actions_in_training;


	time_t startTime;

	set<UID> initiators;
	
	UsersCounts u_counts;
	UsersCounts* userInfl;

	UserList currentSeedSet;
	FloatUserMap Q; // keep marginal gain sorted
	
public:
	PCModel (AnyOption* opt);
	~PCModel ();
	void doAll();
	void scanActionsLog(CoverageTest* const covTest, int flag); // former moveTimeWindow
	unsigned int getMaxWindowSize() {
		return maxWindowSize;
	}
	
	void readActionsInTraining();

	ActionCov* predictPCCov();
	void setDS(HashTreeCube* AM1, map<AID, map<UID, float>* >* NC,  map<AID, UserList>* SeedSets, ActionCov* covs);

private:
	void propagate(ActionLogTuple* tuple); // former insert function
	void propagate1(ActionLogTuple* tuple); // former insert function
	void propagate1Old(ActionLogTuple* tuple); // former insert function

	void propagate2(ActionLogTuple* newTuple, int flag);

	void resetActionQueue();
	void openOutputFiles();
	void closeOutputFiles();
	inline vector<UID>* getNeighbors(UID u) const;
	float getTime() const;

	void computePCCov(Node* v_node);
	float computePCCov(UserList S);
	void updateTC(UID u);
	void computeTC(Node* u_node);
	void mineSeedSet();
	float getMG(UID v);

	void writeInFile(UID v, float cov, float marginal_gain) ;
	
};
}
#endif
