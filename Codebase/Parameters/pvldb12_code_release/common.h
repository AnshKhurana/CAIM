#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <utility>
#include <set>
#include <stack>
#include <sys/resource.h>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <climits>
#include "anyoption.h"
#include <stdint.h>

using namespace std;
using std::ifstream;

typedef int64_t int_64;

typedef uint32_t UID; // userid
typedef uint32_t AID; // actionid 
typedef uint32_t TS; // timestamp
#define TSMAX UINT_MAX

typedef set<UID> UserList;
typedef map<UID, int> UserCounts;
typedef map<UID, int> UsersCounts;
typedef map<UID, UserList> Matrix; // matrix
typedef map<AID, Matrix> Cube; // Cube of <action, user, list_of_users>
typedef map<AID, unsigned int> InfCounts;


const int MAXLINE=256;
const int wordsize = 32;
const unsigned int AVGDELAY = 7862400; // 13 weeks

struct covVals {
	float actualCov; // actual cov
	float modelCov; // coverage by the model in the question
};

typedef map<AID, covVals> ActionCov;


enum PropModels {
	LT, IC, PC 
};

enum GraphType {
	DIRECTED, UNDIRECTED
};

enum Models {
	M1, S1,	M2,	S2,	M3,	S3,	M4,	S4,	C1,	C2, C3, C4, T1, T2, T3, T4, T5
};

enum ModelType {
	TYPE1, TYPE2, TYPE3, TYPE4
};

enum C3Algo {
	MAX, AVGTOP5, MEDIANTOP10
};

inline unsigned int strToInt(string s) {
	unsigned int i;
	istringstream myStream(s);

	if (myStream>>i) {
		return i;
	} else {
		cout << "String " << s << " is not a number." << endl;
		return atoi(s.c_str());
	}
	return i;
}

inline unsigned int strToInt(const char* s) {
	unsigned int i;
	istringstream myStream(s);

	if (myStream>>i) {
		return i;
	} else {
		cout << "String " << s << " is not a number." << endl;
		return atoi(s);
	}
	return i;
}


inline float strToFloat(const char* s) {
	return atof(s);
	float i;
	istringstream myStream(s);

	if (myStream>>i) {
		return i;
	} else {
		cout << "String " << s << " is not a float." << endl;
		return atof(s);
	}
	return i;
}

inline float strToFloat(string s) {
	return atof(s.c_str());
	float i;
	istringstream myStream(s);

	if (myStream>>i) {
		return i;
	} else {
		cout << "String " << s << " is not a float." << endl;
		return atof(s.c_str());
	}
	return i;
}

inline string floatToStr(float f) {
	stringstream ss;
	ss << f;
	return ss.str();
}

inline int_64 strToInt64(string s) {
	int_64 i;
	istringstream myStream(s);

	if (myStream>>i) {
		return i;
	} else {
		cout << "String " << s << " is not a number." << endl;
		exit(1);
	}
	return i;
}

inline string intToStr(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}


struct Metrics {
	int_64 TP;
	int_64 FP;
	int_64 TN;
	int_64 FN;
	int_64 FN_greedy;

	// for predicting time, FP and TN is same as above
	int_64 TP_time;
	int_64 FN_time;
	float AE; // absolute error in predicting time in days
	float SE; // squared error in predicting time in days

	int_64 TP_greedy;

	Metrics() : TP(0), FP(0), TN(0), FN(0), FN_greedy(0), TP_time(0), FN_time(0), AE(0), SE(0), TP_greedy(0) { };

	// called from Evaluate.cc
	Metrics(string& line, std::string::size_type pos) : AE(0), SE(0), TP_greedy(0) {
		string delim = " \t";
		string str;

		// get TP
		std::string::size_type prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		TP = strToInt64(line.substr(prevpos, pos-prevpos));

		// get FP
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		FP = strToInt64(line.substr(prevpos, pos-prevpos));

		// get TN
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		TN = strToInt64(line.substr(prevpos, pos-prevpos));

		// get FN
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		FN = strToInt64(line.substr(prevpos, pos-prevpos));

		// get FN_greedy
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		FN_greedy = strToInt64(line.substr(prevpos, pos-prevpos));

		// get TP_time
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		TP_time = strToInt64(line.substr(prevpos, pos-prevpos));

		// get FN_time
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		FN_time = strToInt64(line.substr(prevpos, pos-prevpos));
	}

	void reset () {
		TP = 0;
		FP = 0;
		FN = 0;
		FN_greedy = 0;
		TN = 0;
		TP_time = 0;
		FN_time = 0;
		AE = 0;
		SE = 0;
		TP_greedy = 0;
	}
	
	void print() {
		cout << TP << " " << FP << " " << TN << " " << FN << " " << FN_greedy << endl;
	}

	void add(Metrics& m) {
		TP += m.TP;
		FP += m.FP;
		TN += m.TN;
		FN += m.FN;
		FN_greedy += m.FN_greedy;
		TP_time += m.TP_time;
		FN_time += m.FN_time;
		AE += m.AE;
		SE += m.SE;
		TP_greedy += m.TP_greedy;
	}
};


struct TimeMetrics {
	int_64 TP_greedy;

	// for predicting time, FP and TN is same as above
	int_64 TP_time;
	int_64 FN_time;

	// for TP_time cases
	float AE_pos; // absolute error in predicting time in days
	float SE_pos; // squared error in predicting time in days

	// for FN_time cases
	float AE_neg; // absolute error in predicting time in days
	float SE_neg; // squared error in predicting time in days

	// for range of observed values, so that normalized root mean squared
	// deviation can be computed
	float t_max; 
	float t_min; 

	// so that coefficient of variation of the RMSD can be computed
	double t_total;

	float tdiff;
	float tdiffPerc;

	TimeMetrics() : TP_greedy(0), TP_time(0), FN_time(0), AE_pos(0), SE_pos(0), AE_neg(0), SE_neg(0), 
					t_max(0), t_min(0), t_total(0), tdiff(0), tdiffPerc(0) { };

	void reset () {
		TP_greedy = 0;
		TP_time = 0;
		FN_time = 0;
		AE_pos = 0;
		SE_pos = 0;
		AE_neg = 0;
		SE_neg = 0;
		t_max = 0;
		t_min = 0;
		t_total = 0;
		tdiff = 0;
		tdiffPerc = 0;
	}
	
};

float getCurrentMemoryUsage();
#endif
