#include "InputParserForPCModel.h"

namespace PartialCredits {

InputParserForPCModel::~InputParserForPCModel() {
	cout << "destructor of InputParserForPCModel called" << endl;
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

InputParserForPCModel::InputParserForPCModel(AnyOption* opt1)  {
	opt = opt1;

	graphFile = opt->getValue("graphFile");
	actionsFile = opt->getValue("actionsFile");
//	actions_file_schema = (const char*) opt->getValue("actions_file_schema");	

	phase = strToInt(opt->getValue("phase"));	

	cout << "graphFile = " << graphFile << endl;
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

	userInfl = new UserCounts();

	cout << "Done initializing InputParserForPCModel" << endl;
}

// its not building a matrix but keeps info for connected nodes only
void InputParserForPCModel::buildAdjacencyMatFromFile() {

	cout << "In buildAdjacencyMatFromFile from file " << graphFile << endl;
	ifstream myfile (graphFile, ios::in);

	//int haveTS = strToInt(opt->getValue("haveTS"));
	int haveTS = 1;

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

			TS tau_u2v = 0;
			TS tau_v2u = 0;
			if (haveTS == 1) {

				// get the tau_u2v
				prevpos = line.find_first_not_of(" \t:", pos);
				pos = line.find_first_of(" \t:", prevpos);

				if (pos == std::string::npos)
					str = line.substr(prevpos);
				else 
					str = line.substr(prevpos, pos-prevpos);
				tau_u2v = strToFloat(str);


				// get the tau_v2u
				prevpos = line.find_first_not_of(" \t:", pos);
				pos = line.find_first_of(" \t:", prevpos);

				if (pos == std::string::npos)
					str = line.substr(prevpos);
				else 
					str = line.substr(prevpos, pos-prevpos);
				tau_v2u = strToFloat(str);

			}

			
			// get the timestamp
			
			TS ts = 0;
			if (haveTS == 1) {
				prevpos = line.find_first_not_of(" \t:", pos);
				pos = line.find_first_of(" \t:", prevpos);

				if (pos == std::string::npos)
					str = line.substr(prevpos);
				else 
					str = line.substr(prevpos, pos-prevpos);
				ts = strToInt(str);
			} 
//			TS ts = 0;



			if (edges % 50000 == 0) {
				cout << "(node1, node2, ts, tau_u2v, tau_v2u, AM size till now, edges till now, mem) = " << u1 << ", " << u2 << ", " << ts << ", " << tau_u2v << ", " << tau_v2u << ", " << AM->size() << ", " << edges << ", " << getCurrentMemoryUsage() << endl;
			}

			if (u1 == u2) {
				cout << "WARNING: Self edge detected for user " << u1 << ". Ignoring!" << endl;
			} else if (u1 > u2) {
				UID temp = u1;
				u1 = u2;
				u2 = temp;
			} 

			FriendsMap* neighbors = NULL;
			if (haveTS == 0 || tau_v2u > 0) { 
				neighbors = AM->find(u1);
				if (neighbors == NULL) {
					neighbors = new FriendsMap();
					neighbors->insert(pair<UID, times>(u2, times(ts, tau_v2u)));
					AM->insert(u1, neighbors);
					edges++;
				} else {
					FriendsMap::iterator it = neighbors->find(u2);
					if (it == neighbors->end()) {
						neighbors->insert(pair<UID, times>(u2, times(ts, tau_v2u)));
						edges++;

					} else {
						//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
					}
				}
			}
			// also add the edges u2->u1 but done allocate Edge class to them
			// .. it is just to find friends efficiently
			if (haveTS == 0 || tau_u2v > 0) { 
				neighbors = AM->find(u2);
				if (neighbors == NULL) {
					neighbors = new FriendsMap();
					neighbors->insert(pair<UID, times>(u1, times(ts, tau_u2v)));
					AM->insert(u2, neighbors);
				} else {
					FriendsMap::iterator it = neighbors->find(u1);
					if (it == neighbors->end()) {
						neighbors->insert(pair<UID, times>(u1, times(ts, tau_u2v)));
					} else {
						//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
					}
				}
			}
		}
		myfile.close();
	} else {
		cout << "Can't open friendship graph file " << graphFile << endl;
	}

	users = AM->size();
	cout << "BuildAdjacencyMatFromFile done" << endl;
//	cout << "Size of friendship graph hashtree is : " << AM->size() << endl;
//	cout << "Number of edges in the friendship graph are: " << edges << endl;
}


void InputParserForPCModel::readUserCountsInfl() {
	
	string usersCountsFileName = opt->getValue("userInflFile");

	cout << "Reading file " << usersCountsFileName << endl;
	ifstream myfile (usersCountsFileName.c_str(), ios::in);

	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			std::string::size_type pos = line.find_first_of(" \t");
			int	prevpos = 0;

			// get the userid
			string str = line.substr(prevpos, pos-prevpos);
			UID u = strToInt(str);

			// get the counts, number of actions u performed
			prevpos = line.find_first_not_of(" \t", pos);
			pos = line.find_first_of(" \t", prevpos);
			int count = strToInt(line.substr(prevpos, pos-prevpos));

			// get the number of actions for which u got influenced by one of
			// its friends within time \pi which is learned in the first phase
			// of training
			prevpos = line.find_first_not_of(" \t", pos);
			pos = line.find_first_of(" \t", prevpos);
			int parentCount = strToInt(line.substr(prevpos, pos-prevpos));

			int infl = (100*parentCount)/count;  // put it int to set precision to 2 places

	//		u_counts[u] = count;
			userInfl->insert(pair<UID, int>(u, infl));
		}
		myfile.close();
	} else {
		cout << "Can't open usersCounts file " << usersCountsFileName << endl;
	}

	cout << "Read usersCounts file " << usersCountsFileName << endl;

}


void InputParserForPCModel::openActionsLog() {
	pActionsLog.open(actionsFile, ios::in);	

	if (!pActionsLog.is_open()) {
		cout << "cant open actions file:" << actionsFile << endl;
		exit(0);
	}
}

void InputParserForPCModel::closeActionsLog() {
	pActionsLog.close();	
}

HashTreeCube* InputParserForPCModel::getAM() {
	return AM;
}


UserCounts* InputParserForPCModel::getUserInfl() {
	return userInfl;
}



}
