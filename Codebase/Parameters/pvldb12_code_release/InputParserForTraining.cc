#include "InputParserForTraining.h"

namespace Train {

InputParserForTraining::~InputParserForTraining() {
	cout << "destructor of InputParserForTraining called" << endl;
	int length = AM->length();

	for (int i=0; i<length; i++) {
		map<UID, FriendsMap*> curRow = AM->getRow(i);

		for (map<UID, FriendsMap*>::iterator j=curRow.begin(); j!=curRow.end(); j++) {
			FriendsMap* curElement = j->second;
			delete curElement;
		}
	}

	delete AM;

	while (freeTuples != NULL) {
		ActionLogTuple* temp = freeTuples;
		freeTuples = freeTuples->next;
		delete temp;
	}

	cout << "memory in AM released, mem: " << getCurrentMemoryUsage() << endl;
}

InputParserForTraining::InputParserForTraining(AnyOption* opt1) {
	opt = opt1;

	phase = strToInt(opt->getValue("phase"));	
	
	int computeUserInf = strToInt(opt->getValue("computeUserInf"));

	if (computeUserInf == 0) {
		graphFile = opt->getValue("graphFile");
		cout << "graphFile = " << graphFile << endl;
	}

	actionsFile = opt->getValue("actionsFile");
	dynamicFG = 1;
//	actions_file_schema = (const char*) opt->getValue("actions_file_schema");	


	cout << "actionsFile = " << actionsFile << endl;	
//	cout << "actions_file_schema: " << actions_file_schema << endl;	

	edges = 0;

	// initialize AM HashTree
//	AM = NULL;
	AM = new HashTreeCube(89041); // i.e. hashtable of size 1024
//	AM = new HashTreeCube(181081); // i.e. hashtable of size 1024
//	cout << "AM of length " << AM->length() << " created" << endl;

	freeTuples = NULL;
	freeTuplesSize = 0;

	cout << "Done initializing InputParserForTraining" << endl;
}

// its not building a matrix but keeps info for connected nodes only
void InputParserForTraining::buildAdjacencyMatFromFile() {

	cout << "In buildAdjacencyMatFromFile from file " << graphFile << endl;
	ifstream myfile (graphFile, ios::in);

	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			std::string::size_type pos = line.find_first_of(" \t:");
			int	prevpos = 0;

			// get the first node
			string str = line.substr(prevpos, pos-prevpos);
			UID u1 = strToInt(str);

			// get the second node
			prevpos = line.find_first_not_of(" \t:", pos);
			pos = line.find_first_of(" \t:", prevpos);
			str = line.substr(prevpos, pos-prevpos);
			UID u2 = strToInt(str);

			// get the timestamp
			prevpos = line.find_first_not_of(" \t:", pos);
			pos = line.find_first_of(" \t:", prevpos);
			
			if (pos == std::string::npos)
				str = line.substr(prevpos);
			else 
				str = line.substr(prevpos, pos-prevpos);

			TS ts = strToInt(str);
			//TS ts = 0;

			if (edges % 50000 == 0) {
				cout << "(node1, node2, ts, AM size till now, edges till now, mem) = " << u1 << ", " << u2 << ", " << ts << ", " << AM->size() << ", " << edges << ", " << getCurrentMemoryUsage() << endl;
			}

			if (u1 == u2) {
				cout << "WARNING: Self edge detected for user " << u1 << ". Ignoring!" << endl;
			} else if (u1 > u2) {
				UID temp = u1;
				u1 = u2;
				u2 = temp;
			} 

			FriendsMap* neighbors = AM->find(u1);
			if (neighbors == NULL) {
				neighbors = new FriendsMap();
				neighbors->insert(pair<UID, Edge*>(u2, new Edge(ts)));
				AM->insert(u1, neighbors);
				edges++;
			} else {
				FriendsMap::iterator it = neighbors->find(u2);
				if (it == neighbors->end()) {
					neighbors->insert(pair<UID, Edge*>(u2, new Edge(ts)));
					edges++;
					
				} else {
				//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
				}
			}

			// also add the edges u2->u1 but done allocate Edge class to them
			// .. it is just to find friends efficiently
			neighbors = AM->find(u2);
			if (neighbors == NULL) {
				neighbors = new FriendsMap();
				neighbors->insert(pair<UID, Edge*>(u1, NULL));
				AM->insert(u2, neighbors);
			} else {
				FriendsMap::iterator it = neighbors->find(u1);
				if (it == neighbors->end()) {
					neighbors->insert(pair<UID, Edge*>(u1, NULL));
				} else {
				//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
				}
			}
		}
		myfile.close();
	} else {
		cout << "Can't open friendship graph file " << graphFile << endl;
	}

	users = AM->size();
	cout << "BuildAdjacencyMatFromFile done" << endl;
	cout << "Size of friendship graph hashtree is : " << AM->size() << endl;
	cout << "Number of edges in the friendship graph are: " << edges << endl;
}


void InputParserForTraining::printAdjacencyMatrix() {
	/*
	for (Matrix::iterator i=AM.begin(); i!=AM.end(); i++) {
		UID u1 = i->first;
		UserList& edgeList = i->second;
		for (UserList::iterator j=edgeList.begin(); j!=edgeList.end(); j++) {
			UID u2 = *j;
			cout << u1 << " " << u2 << endl;
		}
	}*/
}


void InputParserForTraining::openActionsLog() {
	pActionsLog.open(actionsFile, ios::in);	

	if (!pActionsLog.is_open()) {
		cout << "cant open actions file:" << actionsFile << endl;
		exit(0);
	}
}

void InputParserForTraining::closeActionsLog() {
	pActionsLog.close();	
}

HashTreeCube* InputParserForTraining::getAM() {
	return AM;
}


// its not building a matrix but keeps info for connected nodes only
// for Type 4 models
void InputParserForTraining::readTrainingData() {
	cout << "in readTrainingData" << endl;
	unsigned int myedges = 0;

	string basedir = opt->getValue("training_dir") ;


	// now read edgesCounts file
	string edgesCountsFileName = basedir + "/edgesCounts.txt";
	unsigned int edges = 0;

	cout << "Reading edgesCounts file " << edgesCountsFileName << endl;
	ifstream myfile (edgesCountsFileName.c_str(), ios::in);
	string delim = " \t";	
	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			std::string::size_type pos = line.find_first_of(delim);
			int	prevpos = 0;

			// get first user
			string str = line.substr(prevpos, pos-prevpos);
			UID u1 = strToInt(str);

			// get the second user
			prevpos = line.find_first_not_of(delim, pos);
			pos = line.find_first_of(delim, prevpos);
			UID u2 = strToInt(line.substr(prevpos, pos-prevpos));
			
			if (u1 > u2) {
				cout << "something wrong. u > v" << u1 << " " << u2 << endl;
				exit(1);
			}

			++edges;

			Edge* e = new Edge(line, pos, dynamicFG);

			FriendsMap* neighbors = AM->find(u1);
			if (neighbors == NULL) {
				neighbors = new FriendsMap();
				neighbors->insert(pair<UID, Edge*>(u2, e));
				AM->insert(u1, neighbors);
			} else {
				FriendsMap::iterator it = neighbors->find(u2);
				if (it == neighbors->end()) {
					neighbors->insert(pair<UID, Edge*>(u2, e));
				} else {
				//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
				}
			}

			// also add the edges u2->u1 but done allocate Edge class to them
			// .. it is just to find friends efficiently
			neighbors = AM->find(u2);
			if (neighbors == NULL) {
				neighbors = new FriendsMap();
				neighbors->insert(pair<UID, Edge*>(u1, NULL));
				AM->insert(u2, neighbors);
			} else {
				FriendsMap::iterator it = neighbors->find(u1);
				if (it == neighbors->end()) {
					neighbors->insert(pair<UID, Edge*>(u1, NULL));
				} else {
				//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
				}
			}


//			cout << "(node1, node2, AM size till now, edges till now) = " << u << ", " << v << ", " << AM->size() << ", " << edges << endl;
			if (edges % 50000 == 0) {
				cout << "(node1, node2, AM size till now, edges till now, mem) = " << u1 << ", " << u2 << ", " << AM->size() << ", " << edges << ", " << getCurrentMemoryUsage() << endl;
			}


		}
		myfile.close();
	} else {
		cout << "Can't open edgesCounts file " << edgesCountsFileName << endl;
	}

	u_counts.clear();

	users = AM->size();
	cout << "readTrainingData done" << endl;
	cout << "Size of friendship graph hashtree is : " << AM->size() << endl;
	cout << "Number of edges in the friendship graph are: " << edges << "; myedges: " << myedges/2 << endl << endl;
}



int InputParserForTraining::getUserCounts(UID u) {
	return u_counts[u];
}

UsersCounts& InputParserForTraining::getUsersCounts() {
	return u_counts;
}
}
