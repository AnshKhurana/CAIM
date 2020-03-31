#ifndef INPUTPARSERFORPCMODEL_H
#define INPUTPARSERFORPCMODEL_H

#include "common.h"
#include "HashTreeMap.cc"

namespace PartialCredits {

struct times {
	TS edgeTS ; // timestamp at which users become friends
	TS tau ; // avg time of propagation from u to v

	times (TS t1, TS t2) : edgeTS(t1), tau(t2) {} ;
};

typedef map<UID, times> FriendsMap;
typedef HashTreeMap<UID, FriendsMap*> HashTreeCube;



struct ActionLogTuple {
	UID uid;
	AID a;
	TS ts; // timestamp
	UID parentid; // for meme
	ActionLogTuple* next;

	ActionLogTuple (std::string& line) : next(NULL) {
		string delim = " \t";
		std::string::size_type pos = line.find_first_of(delim);
		int	prevpos = 0;
		
		// get the user id 
		uid = strToInt(line.substr(prevpos, pos-prevpos));

		// get the action id
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		a = strToInt(line.substr(prevpos, pos-prevpos));

		// get the timestamp
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		if (pos == string::npos) 
			ts = strToInt(line.substr(prevpos));
		else
			ts = strToInt(line.substr(prevpos, pos-prevpos));

		parentid = 0;
		if (ts != 0 && pos != string::npos) {
			prevpos = line.find_first_not_of(delim, pos);
			pos = line.find_first_of(delim, prevpos);
			if (prevpos != string::npos) {
				if (pos == string::npos) 
					parentid = strToInt(line.substr(prevpos));
				else
					parentid = strToInt(line.substr(prevpos, pos-prevpos));
			}
		}
	}

	void updateTuple (std::string& line) {
		string delim = " \t";
		std::string::size_type pos = line.find_first_of(delim);
		int	prevpos = 0;
		
		unsigned int t1, t2, t3;
		// get the user id 
		uid = strToInt(line.substr(prevpos, pos-prevpos));

		// get the action id
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		a = strToInt(line.substr(prevpos, pos-prevpos));

		// get the timestamp
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		if (pos == string::npos) 
			ts = strToInt(line.substr(prevpos));
		else
			ts = strToInt(line.substr(prevpos, pos-prevpos));

		parentid = 0;
		if (ts != 0 && pos != string::npos) {
			prevpos = line.find_first_not_of(delim, pos);
			pos = line.find_first_of(delim, prevpos);
			if (prevpos != string::npos) {
				if (pos == string::npos) 
					parentid = strToInt(line.substr(prevpos));
				else
					parentid = strToInt(line.substr(prevpos, pos-prevpos));
			}
		}
		
		next = NULL;
	}		

};

typedef map<UID, ActionLogTuple*> UsersTuplesMap;

class InputParserForPCModel {


	const char *graphFile, *actionsFile;
	UID users; // number of users
	AID actions; // number of actions
	int_64 edges;	

	unsigned int maxTuples;
	HashTreeCube* AM;  // adjacency matrix	
//	UsersCounts u_counts; // maintain count of actions each user performs in training set	

	//
	ifstream pActionsLog; // pointer to actions log

	// for flickr
	ActionLogTuple* freeTuples;
	int freeTuplesSize;

	// training of testing
	AnyOption* opt;
	int phase;

	UsersCounts* userInfl;

public:
	InputParserForPCModel(AnyOption* opt);
	~InputParserForPCModel();

	// functions related to friendship graph
	void buildAdjacencyMatFromFile(); // read the graph from file and build adjacency matrix
	void printAdjacencyMatrix();
	void writeGraphClusOutput();

	void readUserCountsInfl();
	UserCounts* getUserInfl() ;
	
	
	// functions related to actions log
	void openActionsLog(); // open actions log file
	void closeActionsLog(); // close actions log file
	inline ActionLogTuple* getNextTuple();
	inline ActionLogTuple* getNewTuple(string& line);

	// function to optimize memory new/free calls
	inline void deleteTuple(ActionLogTuple*);

	HashTreeCube* getAM();

	// testing routined
	void readTrainingData();
	void readUsersList();
	void processAM();
	int getUserCounts(UID u);
	
//	UsersCounts& getUsersCounts();

};


// keeps the neighbors info for u1 for next call
/*
inline Edge* InputParserForPCModel::getEdge(UID u1, UID u2) {

	static FriendsMap* neighbors = NULL;
	static UID rootUser = -1; // the user whose neighbors are in cache
		
	
	if (u2 < u1) {
		FriendsMap* neighbors_v = AM->find(u2);
		if (neighbors_v != NULL) {
			FriendsMap::iterator it = neighbors_v->find(u1);
			if (it != neighbors_v->end()) return (*it).second;
		}
	} else {
		if (rootUser != u1) {
			rootUser = u1;
			neighbors = AM->find(u1);
		}
	
		if (neighbors != NULL) {
			FriendsMap::iterator it = neighbors->find(u2);
			if (it != neighbors->end()) return (*it).second;
		}
		
	}
	return NULL;
}
*/

inline ActionLogTuple* InputParserForPCModel::getNewTuple(string& line) {

	// initialize the current action
	ActionLogTuple* cur = NULL;
	if (freeTuples != NULL) {
		cur = freeTuples;
		freeTuples = freeTuples->next;
		cur->updateTuple(line);
		freeTuplesSize--;
	} else {
		cur = new ActionLogTuple(line);
	}

	return cur;
}


inline ActionLogTuple* InputParserForPCModel::getNextTuple() {
	// Assume that actions log is open, so wont check it here again
	static TS prevTS = 0;
	static AID prevAction = 0;
	
	while ((!pActionsLog.eof())) {
		std::string line;
		getline (pActionsLog,line);
		if (line.empty()) continue;

		// initialize the current action
		ActionLogTuple* cur = NULL;
		if (freeTuples != NULL) {
			cur = freeTuples;
			freeTuples = freeTuples->next;
			cur->updateTuple(line);
			freeTuplesSize--;
		} else {
			cur = new ActionLogTuple(line);
		}

		/*
		if (phase == 1) {
			// checks whether the action log file is sorted in the desired order
			// actions ids should be in decreasing order
			if (cur->a > prevAction && prevAction != 0) {
				cout << "Actions log file is not sorted by actions in increasing order" << endl;
				exit (0);
			}

			// timestamps should be sorted in decreasing order for same action
			if (cur->a == prevAction) {
				if (cur->ts > prevTS && prevTS != 0) {
					cout << "Actions files not sorted by time in decreasing order:" << actionsFile << endl;
					exit(0);
				}
			}
		} else {
			// checks whether the action log file is sorted in the desired order
			// actions ids should be in decreasing order
			if (cur->a < prevAction && prevAction != 0) {
				cout << "Actions log file is not sorted by actions in decreasing order" << endl;
				exit (0);
			}

			// timestamps should be sorted in decreasing order for same action
			if (cur->a == prevAction) {
				if (cur->ts < prevTS && prevTS != 0) {
					cout << "Actions files not sorted by time in increasing order:" << actionsFile << endl;
					exit(0);
				}
			}
		}
*/
		prevTS = cur->ts;
		prevAction = cur->a;
		
		return cur;
		break;
	}
	return NULL; // file ends
}

inline void InputParserForPCModel::deleteTuple(ActionLogTuple* tuple) {
	// keep 100 free tuples in buffer
	if (freeTuplesSize < 65000) {
		tuple->next = freeTuples;
		freeTuples = tuple;
		++freeTuplesSize;
	} else {
		delete tuple;
	}
}
}
#endif
