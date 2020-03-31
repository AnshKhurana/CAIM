#include "PCModel.h"

namespace PartialCredits {
	
PCModel::PCModel(AnyOption* opt1) {

	opt = opt1;

	input = new InputParserForPCModel(opt);

	// initialize private variables
	head = NULL;
	tail = NULL;

	maxTuples = strToInt((const char*) opt->getValue("maxTuples"));	

	int phase = strToInt((const char*) opt->getValue("phase"));

	if (phase == 12) {
		tol = strToFloat((const char*) opt->getValue("truncation_threshold"));	
		budget = strToFloat((const char*) opt->getValue("budget"));	
		cout << "Truncation_threshold: " << tol << endl;
		cout << "Budget: " << budget << endl;
	} else {
		tol = 0.001 ; // not needed, but just using it.
	}

	outdir = opt->getValue("outdir");		
	actionsFile = opt->getValue("actionsFile");
	
	tuplesInCurrentWindow = new UsersTuplesMap();
//	dbc = new DBConnect(opt);

	
	maxWindowSize = 0;
	actionsDone = 0;

	time (&startTime);
}

PCModel::~PCModel() {

	cout << "total time taken : " << getTime() << endl;
	
	resetActionQueue();
}

void PCModel::resetActionQueue() {

	for (UsersTuplesMap::iterator i=tuplesInCurrentWindow->begin(); i!=tuplesInCurrentWindow->end(); ++i) {
		ActionLogTuple* curTuple = i->second;
	//	cout << "deleting tuple for user " << i->first << endl;
		input->deleteTuple(curTuple);
		i->second = NULL;
		
	}

	tuplesInCurrentWindow->clear();
	initiators.clear();

	for (map<UID, map<AID, float>* >::iterator it = TCb.begin(); it!=TCb.end(); it++) {
		map<AID, float>* tcb_y = it->second;
		tcb_y->clear();
		delete tcb_y;
	}

	TCb.clear();

	head = NULL;
	tail = NULL;
}

void PCModel::doAll() {

	
//	readActionsInPCModel();
	
	// 1 for computing CN table counts
	// 2 for filtering action log
	// 3 for preparing propagation log 

	openOutputFiles();
	input->buildAdjacencyMatFromFile();
	readActionsInTraining();
	AM = input->getAM();

	scanActionsLog(NULL, 0);

	mineSeedSet();
//	writeCountsInFile();
	closeOutputFiles();

}



void PCModel::scanActionsLog(PartialCredits::CoverageTest* covTest, int flag) {
	cout << "In function PCModel::scanActionsLog " << endl;
	unsigned long curSize = 0; // size of the current window
	unsigned long thigh = 0; // highest timestamp in the current time window
	unsigned long tcount = 0;
	unsigned long tuplesInPCModel = 0;
	AID prevAction = 0;

	// get User Influence
	input->readUserCountsInfl();
	userInfl = input->getUserInfl();

	ifstream pActionsLog(actionsFile, ios::in);	
	
	while ((!pActionsLog.eof())) {
		std::string line;
		getline (pActionsLog,line);
		if (line.empty()) continue;

		ActionLogTuple* cur = input->getNewTuple(line);


		if (tcount % 10000 == 0) {
			cout << "tuples read, tuples in training, current window size, max window size, current memory, time: " << tcount << ", " << tuplesInPCModel << ", " << curSize << ", " << maxWindowSize << ", " << getCurrentMemoryUsage() << ", " << getTime() << endl;
		}
		
		if (flag != 0 && tcount > 10 && tcount % 50000 == 0) {
//			predictPCCov();
//			covTest->writeOutput(tcount);
		}

		if (actions_in_training.find(cur->a) == actions_in_training.end()) { // actions in training are not in testing
			++tcount;
			input->deleteTuple(cur);
			continue;
		}

		++tuplesInPCModel;

		u_counts[cur->uid]++;

		if (cur->a == prevAction || prevAction == 0) {

			prevAction = cur->a;

		} else {
			// action changed
			++actionsDone;

			if (actionsDone % 50 == 0) {
				cout << "\nAction changed from " << prevAction << " to " << cur->a << "; actionsDone: " << actionsDone << " mem: " << getCurrentMemoryUsage() << endl;
			}

			curSize = 0;
			resetActionQueue();
//			cout << "Released memory from Influence Vectors. Current memory usage: " << getCurrentMemoryUsage() << endl << endl;

			prevAction = cur->a;

		}
			
		if (flag == 0) {
			propagate1(cur);
		} else if (flag == 1 || flag == 2) {
			// PC approach
			propagate2(cur, flag);
//		} else if (flag == 2) {
//			propagateDirectCount(cur, NC, SeedSets);
		}

		++curSize;
		if (maxWindowSize < curSize) maxWindowSize = curSize;

		++tcount;

		if (maxTuples != 0 && tuplesInPCModel > maxTuples) {
			input->deleteTuple(cur);
			cur = NULL;			
			break;
		}

	}

	actionsDone++;
	resetActionQueue();

	input->closeActionsLog();

	cout << "Done function PCModel::scanActionsLog " << endl;
	cout << "tuplesInPCModel: " << tuplesInPCModel << endl;
}




// insert the node in the current time window
// issue a lock, update childdata
void PCModel::propagate(ActionLogTuple* newTuple) {
	
	/*
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
	UID y = newTuple->uid;
//	TSUsersMap* ancestors_y = NULL;
//
	set<UID> parents;
		
	int indegree = 0;

	FriendsMap* neighbors = AM->find(y);

	bool isInitiator = true;

	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID z = j->first;
			TS edge_ts = j->second;

			UsersTuplesMap::iterator z_it = tuplesInCurrentWindow->find(z);

			if (z_it != tuplesInCurrentWindow->end()) {

				TS z_ts = z_it->second->ts;
				
				if (newTuple->ts >= edge_ts && newTuple->ts > z_ts) {
					indegree++;
					parents.insert(z);
					//Ancestors[y].insert(pair<TS, UID>(z_ts, z));
				}
			}

		}
	}

//	UsersAncestorsMap::iterator it = Ancestors.find(y);
	
	if (indegree != 0) {
		float dc = (float)1/indegree;

		//check on dc
		if (dc == 0) {
			cout << "something is wrong, dc is 0 while indegree is " << indegree << endl;
		}

		for (set<UID>::iterator i=parents.begin(); i!=parents.end(); i++) {
			UID v = *i;

			if (TC.find(v) == TC.end()) { 
				UsersTuplesMap* followers_v = new UsersTuplesMap();

				TC[v] = followers_v;
			}

			for(InfCube::iterator j=TC.begin(); j!=TC.end(); j++) {
				UID w = j->first;
				UserActionMap& followers = j->second;
				
				UserActionMap::iterator it = followers.find(v);
				if (it == followers.end()) continue;

				ActionFloatMap::iterator it1 = it->second.find(newTuple->a);
				if (it1 == it->second.end()) continue;

				TC[w][y][newTuple->a] += it1->second * dc;
			}
		}

	}

	

	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(y, newTuple));

	if (isInitiator) {
		initiators.insert(y);
	}

	if (head == NULL) {
		head = newTuple;
		tail = newTuple;
	} else { // append at end
		tail->next = newTuple;
		tail = newTuple;
	}
	*/
	
}



// process one tuple at a time
// compute coverage of all seedSets taken from test set for PC
void PCModel::propagate2(ActionLogTuple* newTuple, int flag) {
	
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
	UID y = newTuple->uid;
	AID a = newTuple->a; 

	map<UID, float> parents;
//	float dc = 0;

	float y_userInfl = 0;
	float sum = 0;

	FriendsMap* neighbors = AM->find(y);


	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID z = j->first;
			TS edge_ts = j->second.edgeTS;
			float meanLifeTime = j->second.tau ;

			UsersTuplesMap::iterator z_it = tuplesInCurrentWindow->find(z);

			if (z_it != tuplesInCurrentWindow->end()) {

				TS z_ts = z_it->second->ts;

				// Give credits according to x/
				// Use e^{-delta_t/meanLifeTime}
				if (newTuple->ts >= edge_ts && newTuple->ts > z_ts) {
					float delta_t = newTuple->ts - z_ts; 
					parents[z] = exp(-delta_t/meanLifeTime);
					sum++; 
				}

				// Give credits according to x/
				// pcTime_linear
				/*					if (newTuple->ts >= edge_ts && newTuple->ts > z_ts) {
									sum += newTuple->ts - z_ts;
									parents[z] = newTuple->ts - z_ts; 
									}
									*/					
				/******* Give credits to parents such that tau <= meanLifeTime 
				  if (newTuple->ts >= edge_ts && newTuple->ts > z_ts && (meanLifeTime != 0 && newTuple->ts - z_ts <= meanLifeTime)) {
				  parents.insert(z);
				  }
				 ********/
			}

		}
	}

	if (parents.size() != 0) {
		// user influenceability. Default is 0
		UsersCounts::iterator it = userInfl->find(y);
		if (it != userInfl->end()) {
			y_userInfl = (float)it->second/100;
		}

		//			dc = (float)1/parents.size() * inf;
	}

	// update NC now 

	map<AID, float>* tcb_y = TCb[y];
//	map<AID, float>::iterator it = TCb.find(y);

	if (tcb_y == NULL) {
		tcb_y =  new map<AID, float>();
		TCb[y] = tcb_y;
	} 
	

	for (map<AID, UserList>::iterator i = SeedSets->begin(); i != SeedSets->end(); ++i) {
		AID b = i->first;
		UserList S = i->second;

		// nc == normalized credits
		map<UID, float>* nc = NULL;
		map<AID, map<UID, float>* >::iterator it = NC->find(b);

		if (it != NC->end()) {
			nc = it->second;
		}

		if (S.find(y) != S.end()) {
			// the user is in the seed set
			// tc = total credits
			// tc_{S,y} (b)

			tcb_y->insert(pair<AID, float>(b, 1));
//			(*tcb_y)[b] = 1;

			if (nc == NULL) {
				nc = new map<UID, float>();
				(*NC)[b] = nc;
			}
			(*nc)[y] += 1;
			continue;
		} 

		if (nc == NULL) continue;

		if (parents.size() > 0) {

			for (map<UID, float>::iterator j=parents.begin(); j!=parents.end(); ++j) {
				UID v = j->first;
				float dc = ((float)(j->second * y_userInfl) ) / parents.size();

//				cout << "a, parent, y, dc: " << newTuple->a << ", " << ", " << j->second << ", " << y_userInfl << ", " << sum << ", " << parents.size() << ", " << v << ", " << y << ", " << dc << endl; 

				map<UID, float>::iterator l=nc->find(v);
				if (l != nc->end()) {
					// the parent v is not influenced by S_b

//					if (flag == 2) {
						// direct count approach
//						(*tcb_y)[b] = 1;
//					} else if (flag == 1) {
						float f = (*TCb[v])[b] * dc;
						if (f <= 0.0001) continue;
						(*tcb_y)[b] += f;
//					}

					if (nc == NULL) {
						nc = new map<UID, float>();
						(*NC)[y] = nc;
					}

					(*nc)[y] += (*tcb_y)[b];

					/*
					if (tcb_y[b] > u_counts[y]) {
						cout << "Something is wrong: (y, tcb_y[b], nc[y], u_counts[y], l->second, dc) = " << y << ", " << tcb_y[b] << ", " << (*nc)[y] << ", " << u_counts[y] << ", " << l->second << ", " << dc << endl;
					}
*/
				}
			}

		}
	}
	

	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(y, newTuple));


	if (head == NULL) {
		head = newTuple;
		tail = newTuple;
	} else { // append at end
		tail->next = newTuple;
		tail = newTuple;
	}
	
	
}


ActionCov* PCModel::predictPCCov()  {
	cout << "In predictPCCov" << endl;

	for (ActionCov::iterator i=covs->begin(); i != covs->end(); ++i) {
		AID a = i->first;

		map<UID, float>* nc = NULL;
		// find the action in NC
		map<AID, map<UID, float>* >::iterator it = NC->find(a);
		if (it != NC->end()) {
			nc = it->second;
		}

	//	cout << "(action, actualCov, #initators, #potential users) = " << a << ", " << i->second.actualCov << ", " << SeedSets->find(a)->second.size() << ", " << nc.size() << endl;

		// compute the cov
		float cov = 0;

		if (nc != NULL) {
			for (map<UID, float>::iterator j=nc->begin(); j!=nc->end(); ++j) {
				UID u = j->first;
				int A_u = u_counts[u];

				cov += j->second / A_u;
			}
		}
		i->second.modelCov = cov;		
		cout << a << " " << SeedSets->find(a)->second.size() << " " << i->second.actualCov << " " << i->second.modelCov << endl;
	}
	cout << "Done predictPCCov" << endl;
	return covs;
}

// for dc = exp / indegree * infl
void PCModel::propagate1(ActionLogTuple* newTuple) {
	
	// for all nodes in current timewindow, check what all nodes are reachable
	// from new node
	// we can use the queue (as it is FIFO)
	
//	float tol = 0.001;

	UID y = newTuple->uid;
	AID a = newTuple->a; 

	Node* y_node = new Node(y);
	IM[y][a] = y_node;
//	TSUsersMap* ancestors_y = NULL;
//
	vector<Node*> parents;
	
	float y_userInfl = 0;
	float sum = 0;

	// dc is different for each parent, hence maintain it in a different data structure
	map<UID, float> parents_dc;

		
//	int indegree = 0;

	FriendsMap* neighbors = AM->find(y);

	bool isInitiator = true;

	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID z = j->first;
			TS edge_ts = j->second.edgeTS;
			float meanLifeTime = j->second.tau ;

			UsersTuplesMap::iterator z_it = tuplesInCurrentWindow->find(z);

			if (z_it != tuplesInCurrentWindow->end()) {

				TS z_ts = z_it->second->ts;
				

				// Give credits according to x/
				// Use e^{-delta_t/meanLifeTime}
				if (newTuple->ts >= edge_ts && newTuple->ts > z_ts) {
					float delta_t = newTuple->ts - z_ts; 
					parents_dc[z] = exp(-delta_t/meanLifeTime);
					sum++; 

					Node* z_node = IM[z][a]; 
					parents.push_back(z_node);
					
				}
					
			}

		}
	}

//	UsersAncestorsMap::iterator it = Ancestors.find(y);

	// compute direct credit
	if (parents.size() != 0) {
		// user influenceability. Default is 0
		// alpha = 1 - infl
		UsersCounts::iterator it = userInfl->find(y);
		if (it != userInfl->end()) {
			y_userInfl = (float)it->second/100;
			//cout << "y, alpha1: " << y << ", " << alpha1 << endl;
		}
//		dc = (float)1/parents.size() * y_userInfl;
	}
	

	// compute total credits now
	// for all parents z of y
	for (vector<Node*>::iterator i=parents.begin(); i!=parents.end(); ++i) {
		Node* z_node = *i;
		// initialize total credit of z on y
		//
		// first compute dc based on exp distribution
		UID z = z_node->uid;
		float dc = (parents_dc[z] * y_userInfl) / (float)parents_dc.size();

		if (dc < tol) continue;

		z_node->children[y_node] += dc;
		y_node->ancestors[z_node] += dc;

		// for all ancestors zp of z
		for(map<Node*, float>::iterator j=z_node->ancestors.begin(); j!=z_node->ancestors.end(); ++j) {
			Node* zp_node = j->first;
			// tc_{zp, y} += dc * tc_{zp, z}
			if (dc * j->second >= tol) {
				zp_node->children[y_node] += dc * j->second;
				y_node->ancestors[zp_node] += dc * j->second;
			}
		}
	}


	tuplesInCurrentWindow->insert(pair<UID, ActionLogTuple*>(y, newTuple));

	if (isInitiator) {
		initiators.insert(y);
	}

	if (head == NULL) {
		head = newTuple;
		tail = newTuple;
	} else { // append at end
		tail->next = newTuple;
		tail = newTuple;
	}
	
	
}



// compute the coverage of the userlist S
// It computes cov(S) = \sum_{a} \sum_{u \in V} 
float PCModel::computePCCov(UserList S) {
/*
	float cov = 0;

	for(UserList::iterator i=S.begin(); i!=S.end(); ++i) {
		UID v = *i;

		InfCube::iterator j = IM.find(v);
		if (j == IM.end()) {
			// the user never performed any action
			continue;
		}

		for (ActionNodeMap::iterator k=j->second.begin(); k!=j->second.end(); k++) {
			// that is, for all actions performed by the user
			AID a = k->first;
			Node* u_node = k->second;
			computePCCov(u_node);

			cov += u_node->cov;

		}
	}
*/	
}





// update TC when node y is added to the seed set
void PCModel::updateTC(UID y) {
	
	InfCube::iterator i = IM.find(y);
	if (i == IM.end()) {
		// the user never performed any action
		return;
	}

	// TCb here is tc_{S,x} (a)
	map<UID, map<AID, float>* >::iterator it = TCb.find(y);
	map<AID, float>* tc1 = NULL;

	if (it != TCb.end()) {
		tc1 = it->second;
	}
	
	// for each action that y performs
	for (ActionNodeMap::iterator j=i->second.begin(); j!=i->second.end(); j++) {
		// that is, for all actions performed by the user
		AID a = j->first;
		Node* y_node = j->second;

		// for each follower u of y
		for (map<Node*, float>::iterator k=y_node->children.begin(); k!=y_node->children.end(); ++k) {
			float tc_yu = k->second;
			Node* u_node = k->first;
			UID u = u_node->uid;

			// for each parent x of y
			for (map<Node*, float>::iterator k=y_node->ancestors.begin(); k!=y_node->ancestors.end(); ++k) {
				float tc_xy = k->second;

				// if u is a follower of x
				Node* x_node  = k->first;
				map<Node*, float>::iterator l = x_node->children.find(u_node); 
				if (l != x_node->children.end()) {
					// x has influence on u, edit it
					l->second -= tc_xy * tc_yu;
					
					// also modify ancestors in u_node->ancestors
					u_node->ancestors[x_node] = l->second;

					// Should we delete the node if the weight < tol?
				}

			}

			// now update tc_{S,u} (a)
			// tc_{S,u} (a) += tc_{y,u}(a) * (1 - tc_{S,y}(a))
			float term2 = 0;
			if (tc1 != NULL) {
				map<AID, float>::iterator jt = tc1->find(a);
				if (jt != tc1->end()) {
					term2 = jt->second;
				}
			}

			// tc_Su
			float pp = tc_yu * (1 - term2);
			map<AID, float>* tc_Su = TCb[u];
			if (tc_Su == NULL) {
				tc_Su = new map<AID, float>();
				tc_Su->insert(pair<AID, float>(a, pp));
				TCb[u] = tc_Su;
			} else {
				(*tc_Su)[a] += pp;
			}

		}
	}

}

// compute the marginal gain of x w.r.t S
float PCModel::getMG(UID x) {
		
	float mg = 0;

	InfCube::iterator i = IM.find(x);
	if (i == IM.end()) {
		// the user never performed any action
		return 1;
	}

	// TCb here is tc_{S,x} (a)
	map<UID, map<AID, float>* >::iterator it = TCb.find(x);
	// tc1[a] == tc_{S,x} (a)
	map<AID, float>* tc1 = NULL;

	if (it != TCb.end()) {
		tc1 = it->second;
	}

	for (ActionNodeMap::iterator j=i->second.begin(); j!=i->second.end(); j++) {
		// that is, for all actions performed by the user
		AID a = j->first;
		Node* x_node = j->second;

		// marginalized marginal gain of the node upon itself is (1 - tc_{S, x})/A_x
		float mg_a = 1/(float)u_counts[x]; // marginal gain for this action

		// for each follower u of x
		for (map<Node*, float>::iterator k=x_node->children.begin(); k!=x_node->children.end(); ++k) {
			UID u = k->first->uid;
			if (currentSeedSet.find(u) == currentSeedSet.end()) {
				float tc_xu = k->second;
				mg_a += tc_xu / (float)u_counts[u];
			}
		}

		if (tc1 != NULL) {
			// term2 = tc1[a] == tc_{S,x} (a)
			// mg_a = marginalized coverage of x on a on graph G-S * (1 - term2)
			map<AID, float>::iterator jt = tc1->find(a);
			if (jt != tc1->end()) {
				float term2 = jt->second;
				mg_a = mg_a * (1 - term2);
			}
		}
		mg += mg_a ;

	}

	return mg;
}

void PCModel::mineSeedSet() {

	cout << "In mineSeedSet. Resetting time." << endl; 

	time(&startTime);

	float cov = 0;
	// 1st iteration
	for(InfCube::iterator i=IM.begin(); i!=IM.end(); ++i) {
		
		UID v = i->first;
		float cov_v = getMG(v);


		// enter it into Q
		Q.insert(pair<float, Qrow>(cov_v, Qrow(v, 0)));
		cout << "COVERAGE(PC): " << v << " " << cov_v << endl;
	//		cout << "(node, action, cov, iteration) " << v_node->uid << ", " << a << " " << v_node->cov << " " << v_node->flagCov << endl;
	}

	cout << "Basic work is done" <<endl;

	// CELF
	while (currentSeedSet.size() < budget && currentSeedSet.size() < u_counts.size() ) {
		FloatUserMap::iterator i = Q.end();	
		i--;
		
		Qrow& qrow = i->second;

//		cout << "AMIT: " << qrow.v << ", " << i->first << ", " << qrow.flag << endl;

		if (qrow.flag == currentSeedSet.size()) {
			currentSeedSet.insert(i->second.v);
			cov += i->first;

			updateTC(i->second.v);

			cout << "New seed picked. (total seeds, seed id, cov, mg, time, mem) = " << currentSeedSet.size() << ", " << i->second.v << ", " << cov << ", " << i->first << ", " << getTime() << ", " << getCurrentMemoryUsage() << endl;
			
			writeInFile(i->second.v, cov, i->first);

			Q.erase(i);
		} else {
			UID v = i->second.v;
			float mg = getMG(v);
			Q.erase(i);
			Q.insert(pair<float, Qrow>(mg, Qrow(v, currentSeedSet.size())));
		}
	}
}



void PCModel::openOutputFiles() {

	string filename = outdir + "/PCCov_" + opt->getValue("maxTuples") + "_" + opt->getValue("truncation_threshold") + ".txt";
	outFile.open (filename.c_str());

	if (outFile.is_open() == false) {
		cout << "Can't open file " << filename << " for writing" << endl;
		exit(1);
	}
	
	
}

void PCModel::closeOutputFiles() {
	outFile.close();
}

void PCModel::readActionsInTraining() {
	cout << "In readActionsInPCModel\n";
	string trainingActionsFile = opt->getValue("trainingActionsFile");
	ifstream myfile (trainingActionsFile.c_str(), ios::in);

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

			actions_in_training.insert(a);
		}
		myfile.close();
	} else {
		cout << "Can't open actions_in_training file " << trainingActionsFile << endl;
	}

	cout << "Done readActionsInPCModel. Number of actions in training: " << actions_in_training.size() << endl;

}

float PCModel::getTime() const {
	time_t curTime;
	time(&curTime);

	float min = ((float)(curTime - startTime))/60;
	return min;
}

void PCModel::setDS(HashTreeCube* AM1, map<AID, map<UID, float>* >* NC1, map<AID, UserList>* SeedSets1, ActionCov* covs1) {
	AM = AM1;
	NC = NC1;
	SeedSets = SeedSets1;
	covs = covs1;
}

void PCModel::writeInFile(UID v, float cov, float marginal_gain) {
	cout << endl << endl << "Picked a seed node: " << v << ", total: " << currentSeedSet.size() << endl;
	outFile << v << " " << cov << " " << marginal_gain << " " << getCurrentMemoryUsage() << " " << getTime() << endl;
	cout << v << " " << cov << " " << marginal_gain << " " << getCurrentMemoryUsage() << " " << getTime() << endl;
	cout << endl << endl;
}

}

