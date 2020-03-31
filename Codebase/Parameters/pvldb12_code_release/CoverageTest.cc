#include "CoverageTest.h"

namespace PartialCredits {
	
CoverageTest::CoverageTest(AnyOption* opt1) {
	opt = opt1;
	countLines = 0;
	input = new InputParserForPCModel(opt);

	// initialize private variables
	head = NULL;
	tail = NULL;

	maxTuples = strToInt((const char*) opt->getValue("maxTuples"));	
	outdir = opt->getValue("outdir");		
	actionsFile = opt->getValue("actionsFile");
	
	tuplesInCurrentWindow = new UsersTuplesMap();
//	dbc = new DBConnect(opt);

	
	maxWindowSize = 0;
	actionsDone = 0;

//	NC = new map<AID, map<UID, float> >;

}

CoverageTest::~CoverageTest() {

	cout << "total time taken : " << getTime() << endl;
	
	resetActionQueue();
}

void CoverageTest::resetActionQueue() {

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

void CoverageTest::setModel() {
	m = opt->getValue("propModel");

	model = PC;
	
	if (m.compare("LT") == 0) {
		model = LT;
		
	} else if (m.compare("IC") == 0) {
		model = IC;
		
	} else if (m.compare("PC") == 0) {
		model = PC;
	}

}


void CoverageTest::readSeedFile() {
	int seeds = 0; 
	AID a = 1; // some random action in the test
	int doAllSeeds = 1;

	//	str budget = intToStr(curSeedSet.size());
	string seedFileName = opt->getValue("seedFileName");

	cout << "Seedfilename: " << seedFileName << endl;

	ifstream myfile (seedFileName.c_str(), ios::in);
	string delim = " \t";	
	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			seeds++;
//			if (seeds == 0) continue; // ignore the first line
			std::string::size_type pos = line.find_first_of(delim);
			int	prevpos = 0;

			UID u = 0;
			// get the user
			pos = line.find_first_of(delim, prevpos);
			if (pos == string::npos) 
				u = strToInt(line.substr(prevpos));
			else
				u = strToInt(line.substr(prevpos, pos-prevpos));

			if (doAllSeeds == 1) {

				if (a > 1) {
					SeedSets[a] = SeedSets[a-1];
				}

				SeedSets[a].insert(u);
				covs[a].actualCov = 100;

				cout << "SeedSet size for action " << a << " is " << SeedSets[a].size() << endl;
				a++;
			} else {
				SeedSets[a].insert(u);
				covs[a].actualCov = 100;
				cout << "SeedSet size for action " << a << " is " << SeedSets[a].size() << endl;
			}
		}
	}

	cout << "Size of seedsets is: " << SeedSets.size() << endl;
	myfile.close();

}



void CoverageTest::doAll() {

	time (&startTime);
	
	setModel();

//	readActionsInCoverageTest();
	
	// 1 for computing CN table counts
	// 2 for filtering action log
	// 3 for preparing propagation log 

	input->buildAdjacencyMatFromFile();
//	input->readUserCountsInfl();
	AM = input->getAM();

	readActionsInTest(); 

	int phase = strToInt(opt->getValue("phase"));
	if (phase == 13) {
		scanActionsLog();
	} else if (phase == 14) {
		readSeedFile();
	}

	int flag = 1;

	if (model == PC) {
		flag = 1;
	} else if (model == IC || model == LT ) {
		flag = 3;
	}

	// call PC coverage
	if (model == PC) {
		PCModel* pcm = new PCModel(opt);
		pcm->readActionsInTraining();
		pcm->setDS(AM, &NC, &SeedSets, &covs);
		pcm->scanActionsLog(this, flag);
		covs = *(pcm->predictPCCov());
		writeOutput(0);
		delete pcm;
	} else if (flag = 3) {
		MC* mc = new MC(opt);
		delete AM;

		mc->readInputData();
		computeMCCov(mc);
		writeOutput(0);
//		string propModel = opt->getValue("propModel");
		delete mc;
	}

}




void CoverageTest::scanActionsLog() {
	cout << "In function CoverageTest::scanActionsLog " << endl;
	unsigned long curSize = 0; // size of the current window
	unsigned long thigh = 0; // highest timestamp in the current time window
	unsigned long tcount = 0;
	unsigned long tuplesInCoverageTest = 0;
	AID prevAction = 0;


	ifstream pActionsLog(actionsFile, ios::in);	
	
	while ((!pActionsLog.eof())) {
		std::string line;
		getline (pActionsLog,line);
		if (line.empty()) continue;

		ActionLogTuple* cur = input->getNewTuple(line);

		

		if (tcount % 10000 == 0) {
			cout << "tuples read, tuples in test, current window size, max window size, current memory, time: " << tcount << ", " << tuplesInCoverageTest << ", " << curSize << ", " << maxWindowSize << ", " << getCurrentMemoryUsage() << ", " << getTime() << endl;
		}
		

		if (actions_in_testing.find(cur->a) == actions_in_testing.end()) { // actions in training are not in testing
			++tcount;
			input->deleteTuple(cur);
			continue;
		}
		
		++tuplesInCoverageTest;

		//u_counts[cur->uid]++;
		covs[cur->a].actualCov++;

		if (cur->a == prevAction || prevAction == 0) {

			prevAction = cur->a;

		} else {
			// action changed
			/*
			if (covs[prevAction].actualCov < 500) {
				covs.erase(prevAction);
				SeedSets.erase(prevAction);
			}
*/
			++actionsDone;

			if (actionsDone % 50 == 0) {
				cout << "\nAction changed from " << prevAction << " to " << cur->a << "; actionsDone: " << actionsDone << " mem: " << getCurrentMemoryUsage() << endl;
			}

			curSize = 0;


			resetActionQueue();
//			cout << "Released memory from Influence Vectors. Current memory usage: " << getCurrentMemoryUsage() << endl << endl;

			prevAction = cur->a;

		}
			
		propagate(cur);

		++curSize;
		if (maxWindowSize < curSize) maxWindowSize = curSize;

		++tcount;

		if (maxTuples != 0 && tcount > maxTuples) {
			input->deleteTuple(cur);
			cur = NULL;			
			break;
		}

	}

	actionsDone++;

	/*
	if (covs[prevAction].actualCov < 500) {
		covs.erase(prevAction);
		SeedSets.erase(prevAction);
	}
*/

	resetActionQueue();

	input->closeActionsLog();

	cout << "Done function CoverageTest::scanActionsLog " << endl;
	cout << "tuplesInCoverageTest: " << tuplesInCoverageTest << endl;
	cout << "Number of actions in test set: " << SeedSets.size() << endl;
	cout << "Covs size: " << covs.size() << endl;
	cout << "SeedSets size: " << SeedSets.size() << endl;
}



void CoverageTest::propagate(ActionLogTuple* newTuple) {
	
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
	UID y = newTuple->uid;
	AID a = newTuple->a; 

	FriendsMap* neighbors = AM->find(y);

	bool isInitiator = true;


	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID z = j->first;
			TS edge_ts = j->second.edgeTS;
			//float meanLifeTime = j->second.tau ;

			UsersTuplesMap::iterator z_it = tuplesInCurrentWindow->find(z);

			if (z_it != tuplesInCurrentWindow->end()) {

				TS z_ts = z_it->second->ts;

				if (newTuple->ts >= edge_ts && newTuple->ts > z_ts) {
					isInitiator = false;
				}
			}


		}
	}

				/*
			// we need to take tau from AM and parent.ts from tuple
			UsersTuplesMap::iterator it1 = tuplesInCurrentWindow->find(newTuple->parentid);

			if (it1 != tuplesInCurrentWindow->end()) {
				ActionLogTuple* parent_tuple = it1->second;

				
				
				FriendsMap* neighbors = AM->find(y);

				if (neighbors != NULL) {
					TS meanLifeTime = 0;

					FriendsMap::iterator it = neighbors->find(newTuple->parentid);

					if (it != neighbors->end()) {
						meanLifeTime = it->second.tau;

						if (meanLifeTime != 0 && newTuple->ts - parent_tuple->ts <= meanLifeTime ) {
							isInitiator = false;
						}
					}
				}
*/

 
	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(y, newTuple));

	if (isInitiator) {
		SeedSets[a].insert(y);
	}

	if (head == NULL) {
		head = newTuple;
		tail = newTuple;
	} else { // append at end
		tail->next = newTuple;
		tail = newTuple;
	}
	
	
}


// compute the cov of probabilistic models
void CoverageTest::computeMCCov(MC* mc) {
	int actionsDone = -1;

	for (map<AID, UserList>::iterator i = SeedSets.begin(); i != SeedSets.end(); ++i) {
		AID b = i->first;
		UserList S = i->second;

		covVals& cv = covs[b];
//		if (cv.actualCov < 30) continue;		

//		if (b != 364) continue;
//		if (S.size() != 1) continue;
		actionsDone++;

//		if (actionsDone < 40) continue;

		float cov = 0;
		if (model == IC) {
			cout << "Computing the cov under IC model for action " << b << ". actionsDone " << actionsDone << " (mem, time) = " << getCurrentMemoryUsage() << ", " << getTime() << endl;
			cov = mc->ICCov(S);
		} else if (model == LT) {
			cout << "Computing the cov under LT model for action " << b << ". actionsDone " << actionsDone << " (mem, time) = " << getCurrentMemoryUsage() << ", " << getTime() << endl;
			cov = mc->LTCov(S);
		} 
		
		cv.modelCov = cov;

		cout << b << " " << S.size() << " " << cv.actualCov << " " << cov << endl;

//		break;

//		if (actionsDone == 5) break;
	}

}



// number of tuples in input
void CoverageTest::writeOutput(unsigned long tuples, ActionCov* covs1) {
		
	cout << "In writeOutput\n";
	
	if (covs1 != NULL) {
		covs = *covs1;
	}

	string m = opt->getValue("propModel"); 

	string filename1 = m + "_" + intToStr(tuples) + ".txt";

	string filename = outdir + "/" + filename1;

	int phase = strToInt(opt->getValue("phase"));
	if (phase == 14) {
		string seedFileName = opt->getValue("seedFileName");
		filename = seedFileName + "_PCCov.txt";
	}
	
	ofstream outFile;
	outFile.open (filename.c_str());

	if (outFile.is_open() == false) {
		cout << "Can't open file " << filename << " for writing" << endl;
	}

	for (ActionCov::iterator i=covs.begin(); i!=covs.end(); ++i) {
//		cout << i->first << " " << i->second.actualCov << " " << i->second.modelCov << endl;
		AID a = i->first;
		int initiators = SeedSets.find(a)->second.size();
		outFile << i->first << " " << initiators << " " << i->second.actualCov << " " << i->second.modelCov << endl;
	}

	outFile.close();

	cout << "Done writeOutput \n";
}


void CoverageTest::readActionsInTest() {
	cout << "In readActionsInTest\n";
	string testingActionsFile = opt->getValue("testingActionsFile");
	ifstream myfile (testingActionsFile.c_str(), ios::in);

	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			std::string::size_type pos = line.find_first_of(" \t:");
			int	prevpos = 0;

			// get the actionid
			string str;
			
			if (pos == std::string::npos)
				str = line.substr(prevpos);
			else 
				str = line.substr(prevpos, pos-prevpos);

			AID a = strToInt(str);

			actions_in_testing.insert(a);
		}
		myfile.close();
	} else {
		cout << "Can't open actions_in_testing file " << testingActionsFile << endl;
	}

	cout << "Done readActionsInTest. Number of actions in test:" << actions_in_testing.size() << endl;

}

float CoverageTest::getTime() const {
	time_t curTime;
	time(&curTime);

	float min = ((float)(curTime - startTime))/60;
	return min;
}

}
