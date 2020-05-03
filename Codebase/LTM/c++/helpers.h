#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <fstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <unordered_set>
#include <time.h>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/graph/topological_sort.hpp>
#include <unordered_map>
#include <ctime>
#include <tuple> 
#include <queue>
#include <math.h>
#include <unistd.h>

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
unordered_map<int, double> save_in_degrees(DiGraph G);
double sigmoid(double x);
DiGraph read_graph(string graph_filename);
DiGraph simple_read_graph(string graph_filename);
void read_features(string feature_filename, DiGraph G, unordered_map<int, vector<int> > &Nf, unordered_map<int, vector<pair<int, int> > > &Ef);
void read_probabilities(string prob_filename, edge_prob &P);
void read_probabilities2(string prob_filename, vector<pair<int, int> > &order, vector<double> &P);
void read_groups(string group_filename, unordered_map<int, unordered_set<int> > &groups);
void read_seeds(string seeds_filename, unordered_set<int> &S, int length);


edge_prob increase_probabilities(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F, vector<pair<int, int> > E, edge_prob &P, unordered_map <int, double> &in_degrees);
edge_prob increase_probabilities_no_transform(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F, vector<pair<int, int> > E, edge_prob &P, unordered_map <int, double> &in_degrees);
void decrease_probabilities(edge_prob changed, edge_prob &P);
edge_prob init_probs(DiGraph, edge_prob, unordered_map <int, double> &in_degrees);
edge_prob init_probs_no_transform(DiGraph, edge_prob, unordered_map <int, double> &in_degrees);
void increase_prob_set(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F,
                                 unordered_map<int, vector<pair<int, int> > > feat_to_edges, edge_prob &P, unordered_map <int, double> &in_degrees);

struct NodeParams{
    
    double threshold;
    double in_weight_sum;
    bool active;
};

