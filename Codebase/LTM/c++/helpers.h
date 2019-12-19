#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <unordered_set>
#include <time.h>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/graph/topological_sort.hpp>
#include <unordered_map>
#include <ctime>
#include <tuple>

using namespace std;

struct vertex_info {int label;};

typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::bidirectionalS> DiGraph;
typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::bidirectionalS, vertex_info> SubGraph;
typedef boost::graph_traits<SubGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<SubGraph>::edge_descriptor edge_t;
typedef boost::graph_traits<DiGraph>::vertex_iterator vertex_iter;
typedef boost::graph_traits<DiGraph>::edge_iterator edge_iter;
typedef boost::graph_traits<DiGraph>::out_edge_iterator out_edge_iter;
typedef boost::graph_traits<DiGraph>::in_edge_iterator in_edge_iter;
typedef boost::unordered_map<pair<int, int>, double> edge_prob;
typedef map<edge_t, double> prob_e;
typedef vector<tuple<int, int, double> > edge_info;

// Prototypes
void print_vertices(DiGraph G);
void print_edges(DiGraph G);
void print_degree(DiGraph G);
void print_node_edges(DiGraph G);
void print_size(DiGraph G);
DiGraph read_graph(string graph_filename);
void read_features(string feature_filename, DiGraph G, unordered_map<int, vector<int> > &Nf, unordered_map<int, vector<pair<int, int> > > &Ef);
void read_probabilities(string prob_filename, edge_prob &P);
void read_probabilities2(string prob_filename, vector<pair<int, int> > &order, vector<double> &P);
void read_groups(string group_filename, unordered_map<int, unordered_set<int> > &groups);
