#ifndef MC_H
#define MC_H

#include "common.h"
#include "HashTreeMap.cc"
#include <vector>
#include <cmath>
#include <ctime>
#include <map>
#include <queue>

namespace _MC {

typedef map<UID, float> FriendsMap;
typedef HashTreeMap<UID, FriendsMap*> HashTreeCube;
typedef map<int, unsigned int> RadiusCovMap;

struct MGStruct {
	UID nodeID; // user ID
	float gain; // MG(u|S)
	UID v_best; // user that gives best MG till now
	
	// marginal gain of user u w.r.t the curSeedSet + u_best. i.e., MG(u|S+v_best)
	float gain_next;
	int flag; // size of the seed set in current run
};

// to make a list sorted by MGs (or coverage)
typedef multimap<float, MGStruct*> Gains; 

struct NodeParams {
	float threshold;
	float inWeight;
	bool active;
};

class MC {

	AnyOption* opt;
	
	UserList curSeedSet; 
	UserList users;
	UserList covBestNode;	 // newly made for binary prob. case 
	//UserList covSeedSet;

	HashTreeCube* AM; 
	FriendsMap seedSetNeighbors;
	Gains mgs;

	string outdir;
	const char* probGraphFile;
	string m;
	PropModels model; 
	GraphType graphType;

	string problem;
	ofstream outFile;
	int startIt;
	int binaryProb; // 1: yes, binary probability. 0: no, real probability.


	int eta;
	int countIterations;

	float totalCov; 
	float tol;

	// parameters to monitor the progress of experiments
	time_t startTime;
	time_t stime_mintime;

	RadiusCovMap rcMap;

public:
	MC (AnyOption* opt);
	~MC ();
	void doAll();
	void readInputData();
	PropModels getModel();
	float LTCov(UserList& list);
	float ICCov(UserList& list);
	

private:
	// functions called from constructor
	void setModel(AnyOption* opt);

	// functions called from doAll
	float mineSeedSet(int t_ub=0);
	void mintime();
	void genMintimeTable();
	float ICCov(UserList& list, int t_ub);
	float ICCovOpt(UID v, int t_ub);
	void ICCovPerRadius(UserList& list, int t_ub);
	void clear();
	void computeCov();

	void updateGraphIC(UID v);
	
	/************ Nov 11  ************/
	bool ICCovNew(MGStruct* pMG, MGStruct* pBestMG);
	float mineSeedSetNew();
	

	// other private functions
	float getTime() const;
	float getTime_cur() const;
	void writeInFile(UID v, float cov, float marginal_gain, int curTimeStep, float actualCov, float actualMG, int countUsers);
	
	void openOutputFiles();
	void openNewOutputFiles(int t_ub);

	void printVector(vector<UID>& vec, float pp);

	void writeCovInFile();
};
}
#endif
