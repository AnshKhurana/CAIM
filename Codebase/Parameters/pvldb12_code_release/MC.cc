#include "MC.h"

namespace _MC {

MC::MC(AnyOption* opt1) {
	opt = opt1;

	cout << "In testing phase" << endl;

	AM = new HashTreeCube(89041);

	outdir = opt->getValue("outdir");	
	probGraphFile = opt->getValue("probGraphFile");
	//eta = strToInt(opt->getValue("eta"));
	graphType = DIRECTED;

	setModel(opt);
	
	countIterations = strToInt(opt->getValue("mcruns"));
//	countIterations = 10000;
	//countIterations = 100;

//	testingActionsFile = (const char*) opt->getValue("testingActionsFile");	
	

	cout << "User specified options: " << endl;
	cout << "model : " << m << " or " << model << endl;
	cout << "outdir : " << outdir << endl;
	cout << "probGraphFile : " << probGraphFile << endl;
	cout << "Number of iterations in MC : " << countIterations << endl;

	time (&startTime);
	srand ( startTime );
	time (&stime_mintime);

}

MC::~MC() {
	cout << "total time taken : " << getTime() << endl;
}


void MC::setModel(AnyOption* opt) {
	m = opt->getValue("propModel");
	
	model = LT;

	if (m.compare("LT") == 0) {
		model = LT;
		
	} else if (m.compare("IC") == 0) {
		model = IC;
		
	} else if (m.compare("PC") == 0) {
		model = PC;
	}
	

}



void MC::doAll() {


	readInputData();

	cout << "Model " << m << " : " << model << " running" << endl;

	int phase = strToInt(opt->getValue("phase"));

	if (phase == 11) {
		computeCov();
	} else {
		//problem = "mintime";
		//problem = "mintimeTable";
		openOutputFiles();
		mineSeedSet(); 
		outFile.close();
	}

}

void MC::computeCov() {
	// read the seed set from the file
	// for each radius R, generate the table 
	// <S_alg, R, cov^R>

	int seeds = 0; 

	//	str budget = intToStr(curSeedSet.size());
	string seedFileName = opt->getValue("seedFileName");
	string propModel = opt->getValue("propModel");
	string filename = seedFileName + "_" + propModel + "cov.txt" ;
	ofstream outFile (filename.c_str(), ios::out);

	cout << "Seedfilename: " << seedFileName << "; outputfile: " << filename << endl;

	ifstream myfile (seedFileName.c_str(), ios::in);
	string delim = " \t";	
	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			seeds++;
			if (seeds == 0) continue; // ignore the first line
			std::string::size_type pos = line.find_first_of(delim);
			int	prevpos = 0;

			UID u = 0;
			// get the user
			pos = line.find_first_of(delim, prevpos);
			if (pos == string::npos) 
				u = strToInt(line.substr(prevpos));
			else
				u = strToInt(line.substr(prevpos, pos-prevpos));

			curSeedSet.insert(u);


			float cov = 0;
			if (model == IC) {
				cout << "Computing the cov under IC model" << endl;
				cov = ICCov(curSeedSet);
			} else if (model == LT) {
				cout << "Computing the cov under LT model" << endl;
				//		float covNew = computeLTCov(curSeedSet);
				cov = LTCov(curSeedSet);

				cout << "(Cov): " << cov << endl;

			}

			cout << "(size of seed set, u, cov) = " << curSeedSet.size() << " " << u << " " << cov << endl;

			outFile << u << " " << cov << endl;

		}
	}

	myfile.close();

	outFile.close();

}



float MC::mineSeedSet(int t_ub) {
	
	clear();
	int phase = strToInt(opt->getValue("phase"));
	
	int budget = strToInt(opt->getValue("budget"));  

	if (t_ub != 0) { 
		budget = 50;
	} 
	
	multimap<float, UID> covQueue; // needed to implement CELF
	totalCov = 0;

	int countUsers = 0 ;

	// first pass, without CELF
	for (UserList::iterator i = users.begin(); i!=users.end(); ++i) {
		UID v = *i;
		UserList S;
		S.insert(v);

		float newCov = 0; 
		
		if (model == IC) {
			//newCov = ICCov(S,t_ub);
			newCov = ICCovOpt(v,t_ub);

			if (phase == 15) {
				cout << "COVERAGE(IC): " << v << " " << newCov << endl;
			}
		} else if (model == LT) {
			newCov = LTCov(S);
			if (phase == 15) {
				cout << "COVERAGE(LT): " << v << " " << newCov << endl;
			}
			
		}

		covQueue.insert(pair<float, UID>(newCov, v));

		countUsers++;

		if (countUsers % 100 == 0) {
			cout << "Number of users done in this iteration: " << countUsers << endl;
			cout << "Time, cov, v : " << getTime() << ", " << totalCov << ", " << *curSeedSet.begin() << endl;
		}

//		if (countUsers % 500 == 0) break;

		if (totalCov < newCov) {
			totalCov = newCov;
			curSeedSet.clear();
			curSeedSet.insert(v);
		} 
		

	} 

	if (phase == 15) {
		exit(0);
	}

	if (curSeedSet.size() != 1) {
		cout << "Some problem" << endl ;
	}

	float actualCov = totalCov;
	if (t_ub != 0) {
		//actualCov = ICCov(curSeedSet, 0);
		actualCov = ICCovOpt(*curSeedSet.begin(), 0);
	}

	float prevActualCov = actualCov;
	updateGraphIC(*curSeedSet.begin());

	writeInFile(*curSeedSet.begin(), totalCov, totalCov, t_ub, actualCov, actualCov, countUsers);
	cout << "Number of nodes examined in this iteration: " << countUsers << endl;
//	cout << "True Cov " << trueCov << endl;
	
	// remove the last element from covQueue
	// we have already picked the best node
	multimap<float, UID>::iterator i = covQueue.end();
	i--;
	covQueue.erase(i);

	// CELF
	countUsers = 0;
	UserList usersExamined; 

//	while (totalCov < eta) {
	while (curSeedSet.size() < budget && curSeedSet.size() < users.size()) {
		bool flag = false;

		multimap<float, UID>::iterator i = covQueue.end();
		i--;

		UID v = i->second;

		curSeedSet.insert(v);
		float newCov = 0;;

		if (usersExamined.find(v) == usersExamined.end()) {
			if (model == IC) {
				newCov = ICCovOpt(v, t_ub);
			} else if (model == LT) {
				newCov = LTCov(curSeedSet);
			}

			usersExamined.insert(v);
			countUsers++;

		} else {
			newCov = i->first + totalCov;
			flag = true;
		}

		
		i--;
		float oldCov_nextElement = i->first;
		UID nextElement = i->second;

		if (flag || newCov - totalCov >= oldCov_nextElement) {
			// pick v in the seed set
	
			actualCov = newCov;
			if (t_ub != 0) {
				//actualCov = ICCovOpt(curSeedSet, 0);
				actualCov = ICCovOpt(v, 0);
			}

			writeInFile(v, newCov, newCov - totalCov, t_ub, actualCov, actualCov - prevActualCov, countUsers);
			cout << "Number of nodes examined in this iteration: " << usersExamined.size() << endl;
//			cout << "True Cov " << trueCov << endl;
			
			updateGraphIC(v);
			prevActualCov = actualCov;

			totalCov = newCov;
			// reset parameters
			countUsers = 1;
			usersExamined.clear();

			i++;
			covQueue.erase(i);
			continue;
		} 

		// move the element to another place
		i++;
		covQueue.erase(i);
		covQueue.insert(pair<float, UID>(newCov - totalCov, v));
		
		// its not in seed set
		curSeedSet.erase(v);
	}

	float trueCov = totalCov;

	cout << "True Cov " << trueCov << endl;

	return totalCov;

}



// t_ub = upper bound on number of time steps
// t_ub = 0 means, no limit
// v is the extra node that might be added to the curSeedSet
float MC::ICCovOpt(UID v, int t_ub) {
	float cov = 0;
	
	if (t_ub < 0) {
		cout << "t_ub is 0" << endl;
		exit(0);
	}

	for (int b = 0; b < countIterations; ++b) {

		queue<UID> Q;
		UserCounts activeNodes;

		// add the nighbors of curSeedSet with time step 1
		for (FriendsMap::iterator j = seedSetNeighbors.begin(); j!=seedSetNeighbors.end(); ++j) {
			UID u = j->first;

			if (curSeedSet.find(u) == curSeedSet.end()) {
				float toss = ((float)(rand() % 1001))/(float)1000;
				float p = j->second;

				if (p >= toss) {
					activeNodes[u] = 1;
					Q.push(u);
				}
			}
		}

		// now add v
		Q.push(v);
		activeNodes[v] = 0;

		while(Q.empty() == false) {
			UID v = Q.front(); 
			
			FriendsMap* neighbors = AM->find(v);
			if (neighbors != NULL) {
				// get the time step when v is activated
				int timeStep_v = activeNodes[v];

				if (t_ub == 0 || timeStep_v < t_ub) {
					for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
						UID u = j->first;

						if (curSeedSet.find(u) == curSeedSet.end() && activeNodes.find(u) == activeNodes.end()) {
							float toss = ((float)(rand() % 1001))/(float)1000;
							float p = j->second;

							if (p >= toss) {
								activeNodes[u] = timeStep_v + 1;
								Q.push(u);
							}
						}
					}
				}
			}
			
			Q.pop(); 

		}

		cov += (float)(activeNodes.size() + curSeedSet.size())/countIterations;

	}

//	cov = cov/countIterations;

//	mgs.add(cov, new MGStruct(v, cov, v_best, cov_next));

	return cov;
}


// t_ub = upper bound on number of time steps
// t_ub = 0 means, no limit
float MC::ICCov(UserList& S, int t_ub) {
	float cov = 0;
	
	if (t_ub < 0) {
		cout << "t_ub is 0" << endl;
		exit(0);
	}

	for (int b = 0; b < countIterations; ++b) {

		queue<UID> Q;
		UserCounts activeNodes;

		for (UserList::iterator i=S.begin(); i!=S.end(); ++i) {
			UID v = *i;
			Q.push(v);
			// add v to activeNodes. The time step is 0
			activeNodes[v] = 0;
		}

		while(Q.empty() == false) {
			UID v = Q.front(); 
			
			FriendsMap* neighbors = AM->find(v);
			if (neighbors != NULL) {
				// get the time step when v is activated
				int timeStep_v = activeNodes[v];

				if (t_ub == 0 || timeStep_v < t_ub) {
					for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
						UID u = j->first;

						if (activeNodes.find(u) == activeNodes.end()) {
							float toss = ((float)(rand() % 1001))/(float)1000;
							float p = j->second;

							if (p >= toss) {
								activeNodes[u] = timeStep_v + 1;
								Q.push(u);
							}
						}
					}
				}
			}
			
			Q.pop(); 

		}

		cov += (float)activeNodes.size()/(float)countIterations;

	}

//	cov = cov/countIterations;
	return cov;
}



float MC::ICCov(UserList& S) {
	float cov = 0;
				
	for (int b = 0; b < countIterations; ++b) {
		// Q is the queue in the depth/breadth first search
		queue<UID> Q;
		// activeNodes is the set of nodes that are activated in the current
		// run of Monte Carlo simulation
		UserList activeNodes;

		// S is the seed set
		// for each seed node v is S, 
		// add it to activeNodes
		// add it to Q as well
		for (UserList::iterator i=S.begin(); i!=S.end(); ++i) {
			UID v = *i;
			Q.push(v);
			activeNodes.insert(v);
		}

		while(Q.empty() == false) {
			UID v = Q.front(); 
			
			// AM is adjacency matrix
			FriendsMap* neighbors = AM->find(v);
			if (neighbors != NULL) {
				for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
					UID u = j->first;

					if (activeNodes.find(u) == activeNodes.end()) {
						float toss = ((float)(rand() % 1001))/(float)1000;
						float p = j->second;

						if (p >= toss) {
							activeNodes.insert(u);
							Q.push(u);
						}
					}

				}
			}
			
			Q.pop(); 

		}

		cov += (float)activeNodes.size()/countIterations;

	}

	// compute two things: cov(S) and cov(S+x)
//	cov = cov/countIterations;
	return cov;
	

}

// update the graph
// delete v from the graph
// treat the seed set as one single node
// maintain its outgoing edges in a separate data structure
void MC::updateGraphIC(UID v) {
	FriendsMap* neighbors = AM->find(v);
	if (neighbors != NULL) {
		for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
			UID u = j->first;
			if (curSeedSet.find(u) == curSeedSet.end()) {
				seedSetNeighbors[u] = 1 - (1 - j->second) * (1 - seedSetNeighbors[u]);
			}
		}
	}
}

void MC::clear() {
	covBestNode.clear();
	curSeedSet.clear();
	seedSetNeighbors.clear();
	totalCov = 0;
}


float MC::LTCov(UserList& S) {
//	cout << "In LTCov" << endl;
	float tol = 0.0001;
	float cov = 0;

//	map<UID, float> ppIn; // what is the prob with which the node is covered
	
	for (int b = 0; b < countIterations; ++b) {
		/* initialize random seed: */

		double cov1 = 0;
		queue<UID> T;
		map<UID, NodeParams> Q;

		cov1 += S.size();
		
		for (UserList::iterator i=S.begin(); i!=S.end(); ++i) {
			UID v = *i;
//			ppIn[v] += 1;

			FriendsMap* neighbors = AM->find(v);
			if (neighbors != NULL) {
				for (FriendsMap::iterator j = neighbors->begin(); j!=neighbors->end(); ++j) {
					UID u = j->first;

					if (S.find(u) == S.end()) {

						if (Q.find(u) == Q.end()) {
							NodeParams& np = Q[u];
							np.active = false;
							np.inWeight = j->second;

							/* generate secret number: */
							np.threshold = ((float)(rand() % 1001))/(float)1000;
							T.push(u);
						} else {
							NodeParams& np = Q[u];
							np.inWeight += j->second;					
						}
					} 
				}
			}
		}

		while (!T.empty()) {
			UID u = T.front();

//			cout << "T.size " << T.size() << endl;

			NodeParams& np = Q.find(u)->second;
			if (np.active == false && np.inWeight >= np.threshold + tol) {
//				ppIn[u] += 1;
				np.active = true;
//				cov++;
				cov1++;


				// add u's neighbors to T
				FriendsMap* neighbors = AM->find(u);
				if (neighbors != NULL) {
					// for each neighbor w of u
					for (FriendsMap::iterator k = neighbors->begin(); k!=neighbors->end(); ++k) {
						UID w = k->first;
						// is w is in S, no need to do anything
						if (S.find(w) != S.end()) continue;

						// if w is not in S, locate it in Q
						map<UID, NodeParams>::iterator it = Q.find(w);

						if (it == Q.end()) {
							// if it is not in Q, then 
							NodeParams& np_w = Q[w];
							np_w.threshold = ((float)(rand() % 1001))/(float)1000;
//							np_w.threshold = (float)rand()/RAND_MAX;
							np_w.active = false;
							np_w.inWeight = k->second;
							T.push(w);
						} else {
							// if w is in Q, then 
							NodeParams& np_w = it->second;
							if (np_w.active == false) {
								T.push(w);
								np_w.inWeight += k->second;

								if (np_w.inWeight - 1 > tol) {
									cout << "Something wrong, the inweight for a node is > 1. (w, inweight) = " << w << ", " << np_w.inWeight - 1<< endl;
								}
							}
						}
					}
				}
			}

			// deletes the first element
			T.pop();
		}
//		cout << "Coverage in this iteration: " << cov1 << endl;
		cov1 = cov1/countIterations;
		cov += cov1;
	}


	// coverage from ppIn
	/*
	float cov1 = 0;

	for (map<UID, float>::iterator i = ppIn.begin(); i!=ppIn.end(); ++i) {
		cov1 += i->second;
//		cout << "ppIN: " << i->first << " " << i->second/countIterations << endl;
	}
	
	cov1 = cov1/countIterations;
	cov = cov/countIterations;
*/

//	cout << "(cov, cov1) = " << cov << ", " << cov1 << endl;

	return cov;
	

}


void MC::printVector(vector<UID>& vec, float pp) {
	cout << "AMIT " << pp << " " ;
	for (vector<UID>::iterator i=vec.begin(); i!=vec.end(); ++i) {

		cout << *i << " ";
	}

	cout << endl;

}


void MC::writeInFile(UID v, float cov, float marginal_gain, int curTimeStep, float actualCov, float actualMG, int countUsers) {
	cout << endl << endl << "Picked a seed node: " << v << ", total: " << curSeedSet.size() << endl;
	outFile << v << " " << cov << " " << marginal_gain << " " << curTimeStep << " " << getCurrentMemoryUsage() << " " << getTime() <<  " " << getTime_cur() << " " << actualCov << " " << actualMG << " " << countUsers << endl;
	cout << v << " " << cov << " " << marginal_gain << " " << curTimeStep << " " << getCurrentMemoryUsage() << " " << getTime() << " " << getTime_cur() <<  " " << actualCov << " " << actualMG << " " << countUsers << endl;
	cout << endl << endl;
}


void MC::writeCovInFile() {

	cout << "In writeCovInFile" << endl;

	string filename = opt->getValue("outdir") + string("/LTCov_") + floatToStr(tol) + ".txt" ;
	ofstream outFile (filename.c_str(), ios::out);
	
	UserList S;
	for(UserList::iterator i=curSeedSet.begin(); i!=curSeedSet.end(); ++i) {
		UID v = *i; 
		S.insert(v);

		float cov = LTCov(S);
		outFile << S.size() << " " << v << " " << cov << endl;
		
	}

	outFile.close();

	cout << "Done writeCovInFile" << endl;
}

void MC::openNewOutputFiles(int t_ub) {

	if (outFile.is_open()) {
		outFile.close(); 
	}
	
	string algorithm = "Greedy";
	problem = "maxinf" ;
	cout << "problem : " << problem << endl;
	string filename = outdir + "/" + problem + "_" + m + "_" + algorithm + "_" + intToStr(t_ub) + ".txt";
	outFile.open (filename.c_str());

	if (outFile.is_open() == false) {
		cout << "Can't open file " << filename << " for writing" << endl;
		exit(1);
	}
}


void MC::openOutputFiles() {

	if (outFile.is_open()) {
		outFile.close(); 
	}
	
	string algorithm = "Greedy";
	problem = "maxinf" ;
	cout << "problem : " << problem << endl;
	string filename = outdir + "/" + problem + "_" + m + "_" + algorithm + "_0" + ".txt";
	outFile.open (filename.c_str());

	if (outFile.is_open() == false) {
		cout << "Can't open file " << filename << " for writing" << endl;
		exit(1);
	}
}

PropModels MC::getModel() {
	return model;
}

float MC::getTime_cur() const {
	time_t curTime;
	time(&curTime);

	float min = ((float)(curTime - stime_mintime))/60;
	return min;
}

float MC::getTime() const {
	time_t curTime;
	time(&curTime);

	float min = ((float)(curTime - startTime))/60;
	return min;
}

void MC::readInputData() {
	cout << "in readInputData for model " << model << endl;

	unsigned int edges = -1;

	string probGraphFile = opt->getValue("probGraphFile");
	cout << "Reading file " << probGraphFile << endl;
	ifstream myfile (probGraphFile.c_str(), ios::in);
	string delim = " \t";	
	if (myfile.is_open()) {
		while (! myfile.eof() )	{
			std::string line;
			getline (myfile,line);
			if (line.empty()) continue;
			edges++;
			if (edges == 0) continue; // ignore the first line
			std::string::size_type pos = line.find_first_of(delim);
			int	prevpos = 0;

			// get first user
			string str = line.substr(prevpos, pos-prevpos);
			UID u1 = strToInt(str);

			// get the second user
			prevpos = line.find_first_not_of(delim, pos);
			pos = line.find_first_of(delim, prevpos);
			UID u2 = strToInt(line.substr(prevpos, pos-prevpos));
			
			// get the parameter
			float parameter1 = 0;
			prevpos = line.find_first_not_of(delim, pos);
			pos = line.find_first_of(delim, prevpos);
			if (pos == string::npos) 
				parameter1 = strToFloat(line.substr(prevpos));
			else
				parameter1 = strToFloat(line.substr(prevpos, pos-prevpos));

			if (parameter1 == 0) continue;

//			parameter1 = parameter1 * (1 - alpha);
//			++edges;
			users.insert(u1);
			users.insert(u2);


			if (edges % 10000 == 0) {
				cout << "(node1, node2, weight,  AM size till now, edges till now, mem) = " << u1 << ", " << u2 << ", " << parameter1 << ", " << AM->size() << ", " << edges << ", " << getCurrentMemoryUsage() << endl;
			}


			FriendsMap* neighbors = AM->find(u1);
			if (neighbors == NULL) {
				neighbors = new FriendsMap();
				neighbors->insert(pair<UID, float>(u2, parameter1));
				AM->insert(u1, neighbors);
			} else {
				FriendsMap::iterator it = neighbors->find(u2);
				if (it == neighbors->end()) {
					neighbors->insert(pair<UID, float>(u2, parameter1));
					
				} else {
				//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
				}
			}

			// also add the edges u2->u1 but done allocate Edge class to them
			// .. it is just to find friends efficiently
			if (graphType == UNDIRECTED) { 
				neighbors = AM->find(u2);
				if (neighbors == NULL) {
					neighbors = new FriendsMap();
					neighbors->insert(pair<UID, float>(u1, parameter1 ));
					AM->insert(u2, neighbors);
				} else {
					FriendsMap::iterator it = neighbors->find(u1);
					if (it == neighbors->end()) {
						neighbors->insert(pair<UID, float>(u1, parameter1 ));
					} else {
						//	cout << "WARNING: Edge redundant between users " << u1 << " and " << u2 << endl;
					}
				}
			}
		}
		myfile.close();
	} else {
		cout << "Can't open friendship graph file " << probGraphFile << endl;
	}

	cout << "BuildAdjacencyMatFromFile done" << endl;
	cout << "Size of friendship graph hashtree is : " << AM->size() << endl;
	cout << "Number of users are: " << users.size() << endl;
	cout << "Number of edges in the friendship graph are: " << edges << endl;
}
			


}
