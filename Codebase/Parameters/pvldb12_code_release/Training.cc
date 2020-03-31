#include "Training.h"

namespace Train {
	
Training::Training(AnyOption* opt) {
	countLines = 0;
	input = new InputParserForTraining(opt);

	// initialize private variables
	head = NULL;
	tail = NULL;

	maxTuples = strToInt((const char*) opt->getValue("maxTuples"));	
	outdir = opt->getValue("outdir");		
	trainingActionsFile = (const char*) opt->getValue("trainingActionsFile");	
	actionsFile = opt->getValue("actionsFile");
	computeUserInf = strToInt(opt->getValue("computeUserInf"));
	
//	cout << "graph table name : " << GraphTableName << endl;

//	pi = strToInt((const char*) opt->getValue("pi"));
	// pi != 0 means type 2 models else type 1 or 3 models
	pi = TSMAX;

	tuplesInCurrentWindow = new UsersTuplesMap();
//	dbc = new DBConnect(opt);

	
	maxWindowSize = 0;
	actionsDone = 0;

}

Training::~Training() {

	cout << "total time taken : " << getTime() << endl;
	
	resetActionQueue();
}

void Training::resetActionQueue() {

	for (UsersTuplesMap::iterator i=tuplesInCurrentWindow->begin(); i!=tuplesInCurrentWindow->end(); ++i) {
		ActionLogTuple* curTuple = i->second;
	//	cout << "deleting tuple for user " << i->first << endl;
		input->deleteTuple(curTuple);
		i->second = NULL;
		
	}

	tuplesInCurrentWindow->clear();

	head = NULL;
	tail = NULL;
}

void Training::doAll() {

	time (&startTime);
	
	modelType = 1;
	readActionsInTraining();
	openOutputFiles();

	if (computeUserInf == 1 || modelType == 4) {
		input->readTrainingData();
	} else {
		input->buildAdjacencyMatFromFile();
	}

	AM = input->getAM();
	scanActionsLog();

	if (computeUserInf == 0) {
		writeCountsInFile();
	} else {
		writeCountsInFileNew();
	}

	closeOutputFiles();
	
}


void Training::scanActionsLog() {
	cout << "In function Training::scanActionsLog " << endl;
	unsigned long curSize = 0; // size of the current window
	unsigned long thigh = 0; // highest timestamp in the current time window
	unsigned long tcount = 0;
	unsigned long tuplesInTraining = 0;
	AID prevAction = 0;


	ifstream pActionsLog(actionsFile, ios::in);	
	
	while ((!pActionsLog.eof())) {
		std::string line;
		getline (pActionsLog,line);
		if (line.empty()) continue;

		ActionLogTuple* cur = input->getNewTuple(line);

//		debug = 0;

		if (tcount % 5000 == 0) {
			cout << "tuples read, tuples in training, current window size, max window size, current memory, time: " << tcount << ", " << tuplesInTraining << ", " << curSize << ", " << maxWindowSize << ", " << getCurrentMemoryUsage() << ", " << getTime() << endl;
		}
		
		if (actions_in_training.find(cur->a) == actions_in_training.end()) {
			++tcount;
			input->deleteTuple(cur);
			continue;
		}

		++tuplesInTraining;

		// update users counts
		UsersCounts::iterator it = u_counts.find(cur->uid);
		if (it == u_counts.end()) {
			u_counts[cur->uid] = 1;
		} else {
			it->second++;
		}

		if (cur->a == prevAction || prevAction == 0) {
			if (head != NULL) {
				if (cur->ts - head->ts > pi) {
					// remove nodes/actions corresponding to lowest timestamp
					//cout << "time window width is higher than pi (width, thigh, cur->ts) = " 
					//	<< (thigh - cur->ts) << ", " << thigh << ", " << cur->ts << endl;
					curSize -= update(cur->ts);
				}
			}

			prevAction = cur->a;

		} else {
			// action changed
			++actionsDone;

			cout << "\nAction changed from " << prevAction << " to " << cur->a << "; actionsDone: " << actionsDone << endl;

			curSize = 0;
			resetActionQueue();
//			cout << "Released memory from Influence Vectors. Current memory usage: " << getCurrentMemoryUsage() << endl << endl;

			prevAction = cur->a;

		}

		if (computeUserInf == 1) {
			computeUserInfluencability(cur);
		} else if (modelType == 4) {
			propagateType4Models(cur);
		} else {
			propagate(cur);
		}

		++curSize;
		if (maxWindowSize < curSize) maxWindowSize = curSize;

		++tcount;

		if (maxTuples != 0 && tcount > maxTuples) {
			input->deleteTuple(cur);
			cur = NULL;			
			break;
		}

	}
	resetActionQueue();

	input->closeActionsLog();

	cout << "Done function Training::scanActionsLog " << endl;
	cout << "tuplesInTraining: " << tuplesInTraining << endl;
}

// insert the node in the current time window
// issue a lock, update childdata
void Training::propagate(ActionLogTuple* newTuple) {
	
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
	UID b = newTuple->uid;
	TS b_ts = newTuple->ts;

	FriendsMap* neighbors = AM->find(b);
	newTuple->fmap = neighbors;

	float count_neighbors = 0;

	map<UID, Edge*> myEdges; // a, Edge*

	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID a = j->first;

			UsersTuplesMap::iterator a_it = tuplesInCurrentWindow->find(a);

			if (a_it != tuplesInCurrentWindow->end()) {
				ActionLogTuple* a_tuple = a_it->second;

				Edge* e = j->second;


				if (b > a) { 
					// then e is null and we have to lookup for v->u edge in AM
					FriendsMap* a_neighbors = a_tuple->fmap;
					if (a_neighbors != NULL) {
						e = a_neighbors->find(b)->second;
					} else {
						cout << "v_neighbors is null, something wrong" << endl;
						exit(1);
					}
				}
				
				++e->u_and_v;
				if (dynamicFG == 1 && a_tuple->ts <= e->ts) continue; // edge formed later than the action, so propagation happened here

				TS delta_t = b_ts - a_tuple->ts;

				if (delta_t > 0) { // to avoid delta_t == 0 cases

					if (b < a) {
						// newTuple is u and curTuple is v
						++e->v2u;
						e->delta_t_v2u += delta_t;
					} else {
						// newTuple is v and curTuple is u
						++e->u2v;
						e->delta_t_u2v += delta_t;
					}
					++count_neighbors;

					myEdges.insert(pair<UID, Edge*>(a, e));
				}
			}
		}

		for (map<UID, Edge*>::iterator j=myEdges.begin(); j!=myEdges.end(); ++j) {
			UID a = j->first;
			Edge* e = j->second;

			if (b < a) {
				// newTuple is u and curTuple is v
				e->v2u_credit += (float)1/(count_neighbors);
			} else {
				// newTuple is v and curTuple is u
				e->u2v_credit += (float)1/(count_neighbors);
			}

		}
	}


	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(b, newTuple));

	// update the queue
	// insert the element at the last

	if (head == NULL) {
		head = newTuple;
		tail = newTuple;
	} else { // append at end
		tail->next = newTuple;
		tail = newTuple;
	}
	
	
}


unsigned long Training::update(TS thigh) {
	int count = 0;

	while(head && (thigh - head->ts > pi)) {
		ActionLogTuple* cur = head;
		UID userId = cur->uid;

		UsersTuplesMap::iterator i = tuplesInCurrentWindow->find(userId);

		if (i == tuplesInCurrentWindow->end()) {
			cout << "Something wrong, user performed an action twice. u, a: " << userId << ", " << cur->a << endl;
		} else {
			i->second = NULL;
			tuplesInCurrentWindow->erase(i); // O(1) op
		}

		// update head of the corresponding queue
		head = cur->next;

		input->deleteTuple(cur);
	
	//	if (debug > 1) cout << "\ndeleting node (uid, a)" << cur->uid << ", " << cur->actionId << endl;

		++count;
		//debug = 0;
	}

	if (head == NULL) tail = NULL;

	return count;
}


// insert the node in the current time window
// issue a lock, update childdata
void Training::propagateType4Models(ActionLogTuple* newTuple) {
	
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
	UID b = newTuple->uid;
	TS b_ts = newTuple->ts;

	FriendsMap* neighbors = AM->find(b);
	newTuple->fmap = neighbors;

	float count_neighbors = 0;

	map<UID, Edge*> myEdges; // a, Edge*

	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID a = j->first;

			UsersTuplesMap::iterator a_it = tuplesInCurrentWindow->find(a);

			if (a_it != tuplesInCurrentWindow->end()) {
				ActionLogTuple* a_tuple = a_it->second;

				Edge* e = j->second;

				if (b > a) { 
					// then e is null and we have to lookup for v->u edge in AM
					FriendsMap* a_neighbors = a_tuple->fmap;
					if (a_neighbors != NULL) {
						e = a_neighbors->find(b)->second;
					} else {
						cout << "v_neighbors is null, something wrong" << endl;
						exit(1);
					}
				}
				
				++e->u_and_v;
				if (dynamicFG == 1 && a_tuple->ts <= e->ts) continue; // edge formed later than the action, so propagation didnt happen here

				TS delta_t = b_ts - a_tuple->ts;

				if (delta_t > 0) { // to avoid delta_t == 0 cases

					if (b < a && delta_t <= e->delta_t_v2u) {
						// newTuple is u and curTuple is v
						++e->v2u;
						++count_neighbors;
						myEdges.insert(pair<UID, Edge*>(a, e));
					} else if (a < b && delta_t <= e->delta_t_u2v) {
						// newTuple is v and curTuple is u
						++e->u2v;
						++count_neighbors;
						myEdges.insert(pair<UID, Edge*>(a, e));
					}

				}
			}
		}

		for (map<UID, Edge*>::iterator j=myEdges.begin(); j!=myEdges.end(); ++j) {
			UID a = j->first;
			Edge* e = j->second;

			if (b < a) {
				// newTuple is u and curTuple is v
				e->v2u_credit += (float)1/(count_neighbors);
			} else {
				// newTuple is v and curTuple is u
				e->u2v_credit += (float)1/(count_neighbors);
			}

		}
	}


	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(b, newTuple));

	
}


// insert the node in the current time window
// issue a lock, update childdata
void Training::computeUserInfluencability(ActionLogTuple* newTuple) {
	
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
	UID b = newTuple->uid;
	TS b_ts = newTuple->ts;

	FriendsMap* neighbors = AM->find(b);
	newTuple->fmap = neighbors;

	float count_neighbors = 0;

	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID a = j->first;

			UsersTuplesMap::iterator a_it = tuplesInCurrentWindow->find(a);

			if (a_it == tuplesInCurrentWindow->end()) continue; 
			ActionLogTuple* a_tuple = a_it->second;

			Edge* e = j->second;

			if (b > a) { 
				// then e is null and we have to lookup for v->u edge in AM
				FriendsMap* a_neighbors = a_tuple->fmap;
				if (a_neighbors != NULL) {
					e = a_neighbors->find(b)->second;
				} else {
					cout << "v_neighbors is null, something wrong" << endl;
					exit(1);
				}
			}

			++e->u_and_v;
			if (dynamicFG == 1 && a_tuple->ts <= e->ts) continue; // edge formed later than the action, so propagation didnt happen here

			TS delta_t = b_ts - a_tuple->ts;

			if (delta_t > 0) { // to avoid delta_t == 0 cases

				if (b < a && delta_t <= e->delta_t_v2u) {
					// newTuple is u and curTuple is v
					++count_neighbors;
//					break;
				} else if (a < b && delta_t <= e->delta_t_u2v) {
					// newTuple is v and curTuple is u
					++count_neighbors;
//					break;
				}

			}
		}
	}

	if (count_neighbors > 0) {
		UsersCounts::iterator it = countParents.find(b);
		if (it == countParents.end()) {
			countParents[b] = 1;
		} else {
			it->second++;
		}
	}

	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(b, newTuple));
	
}

void Training::openOutputFiles() {

	string usersCountsFileName = outdir + "/usersCounts.txt";
	usersCountsFile.open (usersCountsFileName.c_str());

	edgesCountsFileName = outdir + "/edgesCounts.txt";
	edgesCountsFile.open (edgesCountsFileName.c_str());

	if (usersCountsFile.is_open() == false) {
		cout << "Can't open file " << usersCountsFileName << " for writing" << endl;
		exit(1);
	}
	
	if (edgesCountsFile.is_open() == false) {
		cout << "Can't open file " << edgesCountsFileName << " for writing" << endl;
	}
	
}

void Training::closeOutputFiles() {
	usersCountsFile.close();
	edgesCountsFile.close();
}

void Training::writeCountsInFileNew() {
	cout << "In writeCountsInFileNew\n";


	for (UsersCounts::iterator it=countParents.begin(); it!=countParents.end(); it++) {
		UID u = it->first;
		int countP = it->second;
		int done = u_counts[u];
		float inf = (float)countP/(float)done;
		usersCountsFile << u << " " << done << " " << countP << " " << inf << endl;
	}

	cout << "Usercounts written, now writing edgesCounts" << endl;

	int countRows = AM->length();
	for (int i=0; i<countRows; i++) {
		map<UID, FriendsMap*> curRow = AM->getRow(i);

		for (map<UID, FriendsMap*>::iterator it = curRow.begin(); it != curRow.end(); it++) {
			UID u = it->first;
			FriendsMap* fmap = it->second;

			for (FriendsMap::iterator j = fmap->begin(); j != fmap->end(); j++) {
				UID v = j->first;

				if (u >= v) continue;

				Edge* e = j->second;

				//edgesCountsFile << u << " " << v << " " << e->u_and_v << endl;
				edgesCountsFile << u << " " << v << " " << e->delta_t_u2v << " " << e->delta_t_v2u << " " << e->ts << endl;
			}

		}
	}
	cout << "Done writeCountsInFileNew\n";
}

void Training::writeCountsInFile() {
	cout << "In writeCountsInFile\n";

	for (UsersCounts::iterator it=u_counts.begin(); it!=u_counts.end(); it++) {
		usersCountsFile << (*it).first << " " << (*it).second << endl;
	}

	cout << "Usercounts written, now writing edgesCounts" << endl;

	int countRows = AM->length();
	for (int i=0; i<countRows; i++) {
		map<UID, FriendsMap*> curRow = AM->getRow(i);

		for (map<UID, FriendsMap*>::iterator it = curRow.begin(); it != curRow.end(); it++) {
			UID u = it->first;
			FriendsMap* fmap = it->second;

			for (FriendsMap::iterator j = fmap->begin(); j != fmap->end(); j++) {
				UID v = j->first;

				if (u >= v) continue;

				Edge* e = j->second;

				edgesCountsFile << u << " " << v << " " << e->u2v << " " << e->v2u << " " << e->u_and_v << " " << e->delta_t_u2v << " " << e->delta_t_v2u << " " << e->u2v_credit << " " << e->v2u_credit << " " << e->ts << endl;
			}

		}
	}

	cout << "Done writeCountsInFile\n";
}

void Training::readActionsInTraining() {
	cout << "In readActionsInTraining\n";
	ifstream myfile (trainingActionsFile, ios::in);

	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			std::string::size_type pos = line.find_first_of(" \t\n:");
			int	prevpos = 0;

			// get the actionid
			string str;
			
			if (pos == std::string::npos)
				str = line.substr(prevpos);
			else 
				str = line.substr(prevpos, pos-prevpos);

			AID a = strToInt(str);

			actions_in_training.insert(a);
		}
		myfile.close();
	} else {
		cout << "Can't open actions_in_training file " << trainingActionsFile << endl;
	}

	cout << "Done readActionsInTraining" << endl;

}

float Training::getTime() const {
	time_t curTime;
	time(&curTime);

	float min = ((float)(curTime - startTime))/60;
	return min;
}

}

