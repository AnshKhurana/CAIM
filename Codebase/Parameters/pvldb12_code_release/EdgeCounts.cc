#include "EdgeCounts.h"

EdgeCounts::~EdgeCounts() {}

EdgeCounts::EdgeCounts(TS ts1) : ts(ts1), u2v(0), v2u(0), u_and_v(0), delta_t_u2v(0),
	delta_t_v2u(0), u2v_credit(0), v2u_credit(0) {}

EdgeCounts::EdgeCounts() : ts(TSMAX), u2v(0), v2u(0), u_and_v(0), delta_t_u2v(0),
	delta_t_v2u(0), u2v_credit(0), v2u_credit(0)
{}

EdgeCounts::EdgeCounts(int _u2v, int _v2u, int _u_and_v, int_64 _delta_t_u2v, int_64 _delta_t_v2u, float _u2v_credit, float _v2u_credit) :
	ts(TSMAX), u2v(_u2v), v2u(_v2u), u_and_v(_u_and_v), delta_t_u2v(_delta_t_u2v), 
	delta_t_v2u(_delta_t_v2u), u2v_credit(_u2v_credit), v2u_credit(_v2u_credit)
{}	

EdgeCounts::EdgeCounts(string& line, std::string::size_type pos) {
	string delim = " \t";
	string str;

	// get u2v
	std::string::size_type prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	u2v = strToInt(line.substr(prevpos, pos-prevpos));

	// get v2u
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	v2u = strToInt(line.substr(prevpos, pos-prevpos));

	// get u_and_v
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	u_and_v = strToInt(line.substr(prevpos, pos-prevpos));

	// get delta_t_u2v
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	delta_t_u2v = strToInt64(line.substr(prevpos, pos-prevpos));

	// get delta_t_v2u
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	delta_t_v2u = strToInt64(line.substr(prevpos, pos-prevpos));

	// get u2v_credit
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	u2v_credit = strToFloat(line.substr(prevpos, pos-prevpos));
	
	// get v2u_credit
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	v2u_credit = strToFloat(line.substr(prevpos, pos-prevpos));
	
	// get ts
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	
	if (pos == std::string::npos)
		str = line.substr(prevpos);
	else 
		str = line.substr(prevpos, pos-prevpos);

	ts = strToInt(str);
}	

EdgeCounts::EdgeCounts(string& line, std::string::size_type pos, int dynamicFG) :
	u2v(0), v2u(0), u_and_v(0), u2v_credit(0), v2u_credit(0)
{
	string delim = " \t";
	string str;

	// get u2v
	std::string::size_type prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
//	u2v = strToInt(line.substr(prevpos, pos-prevpos));

	// get v2u
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
//	v2u = strToInt(line.substr(prevpos, pos-prevpos));

	// get u_and_v
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
//	u_and_v = strToInt(line.substr(prevpos, pos-prevpos));

	// get delta_t_u2v
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	delta_t_u2v = strToInt64(line.substr(prevpos, pos-prevpos));

	// get delta_t_v2u
	prevpos = line.find_first_not_of(delim, pos);
	pos = line.find_first_of(delim, prevpos);
	delta_t_v2u = strToInt64(line.substr(prevpos, pos-prevpos));

	if (dynamicFG == 1) {
		// get u2v_credit
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		//	u2v_credit = strToInt(line.substr(prevpos, pos-prevpos));

		// get v2u_credit
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);
		//	v2u_credit = strToInt(line.substr(prevpos, pos-prevpos));

		// get ts
		prevpos = line.find_first_not_of(delim, pos);
		pos = line.find_first_of(delim, prevpos);

		if (pos == std::string::npos)
			str = line.substr(prevpos);
		else 
			str = line.substr(prevpos, pos-prevpos);

		ts = strToInt(str);
	}
}	


void EdgeCounts::increment_u2v() { // v is a follower of u
	u2v++;
}

void EdgeCounts::increment_v2u() {
	v2u++;
}

void EdgeCounts::increment_u_and_v() {
	u_and_v++;
}

void EdgeCounts::increment_delta_t_u2v(TS delta_t) {
	delta_t_u2v += delta_t;
}

void EdgeCounts::increment_delta_t_v2u(TS delta_t) {
	delta_t_v2u += delta_t;
}

int EdgeCounts::get_u2v() const {
	return u2v;
}

int EdgeCounts::get_v2u() const {
	return v2u;
}

int EdgeCounts::get_u_and_v() const {
	return u_and_v;
}

int_64 EdgeCounts::get_delta_t_u2v() const {
	return delta_t_u2v;
}

int_64 EdgeCounts::get_delta_t_v2u() const {
	return delta_t_v2u;
}

void EdgeCounts::print() const {
	cout << u2v << ", " << v2u << ", " << u_and_v << ", " << delta_t_u2v << ", " << delta_t_v2u << ", " << u2v_credit << ", " << v2u_credit << ", " << ts << endl;
}
