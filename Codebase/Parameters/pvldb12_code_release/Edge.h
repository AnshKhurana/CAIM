#ifndef EDGE_H 
#define EDGE_H

#include "common.h"
#include <string>

class Edge {
public:
	TS ts; // time when two users become friends
	
	// for all models : M1, S1 and C1
	// for S1, use time window of size pi
	// for M1, use time window of size infinity
	int u2v; // number of actions v followed u
	int v2u; // number of actions u followed v
	
	// for all models : M1, S1 and C1
	// number of action both u and v performs
	// by #u, #v and u_and_v, we can compute :
	// number of actions u performs but not v AND
	// number of actions v performs but not u AND
	// number of actions u or v performs
	int u_and_v; 
//	int only_u; // number of actions u performs but not v
//	int only_v; // number of actions v performs but not u

	// for Time-Continuous Models : C1
	int_64 delta_t_u2v; // total time difference for all actions in which v followed u
	int_64 delta_t_v2u; // total time difference for all actions in which u followed v

	// metrics for partial credits
	float u2v_credit;
	float v2u_credit;

	Edge();
	Edge(TS ts);
	Edge(string& line, std::string::size_type pos);
	Edge(string& line, std::string::size_type pos, int dynamicFG);
	Edge(int u2v, int v2u, int u_and_v, int_64 delta_t_u2v, int_64 delta_t_v2u, float u2v_credit, float v2u_credit);
	~Edge();
	void print() const;

	void increment_u2v(); // v is a follower of u
	void increment_v2u();
	
	void increment_u_and_v();
	void increment_delta_t_u2v(TS delta_t);
	void increment_delta_t_v2u(TS delta_t);

	int get_u2v() const;
	int get_v2u() const;
	int get_u_and_v() const ;
	int_64 get_delta_t_u2v() const;
	int_64 get_delta_t_v2u() const;
};

#endif
