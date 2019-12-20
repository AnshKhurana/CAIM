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
#include <queue>
#include <string>
#include <ctime>
#include <tuple>

using namespace std;

struct vertex_info {int label;}; // also have vertex thresholds?

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



DiGraph read_graph(string graph_filename) {
    cout << graph_filename << endl;
    ifstream infile(graph_filename);
    // if (infile==NULL){
        if (! infile.is_open()){
        cout << "Unable to open the input file\n";
    }

    unordered_map<int, int> unordered_mapped;
    int u, v;
    int node_count=0;
    pair<DiGraph::edge_descriptor, bool> edge_insertion;
    DiGraph G;

    while (infile >> u >> v) {
        if (unordered_mapped.find(u) == unordered_mapped.end()) {
            unordered_mapped[u] = node_count;
            node_count++;
        }
        if (unordered_mapped.find(v) == unordered_mapped.end()) {
            unordered_mapped[v] = node_count;
            node_count++;
        }
        edge_insertion=boost::add_edge(unordered_mapped[u], unordered_mapped[v], G);
        if (!edge_insertion.second) {
            std::cout << "Unable to insert edge\n";
        }
    }
    return G;
}

void read_features(string feature_filename, DiGraph G, unordered_map<int, vector<int> > &Nf, unordered_map<int, vector<pair<int, int> > > &Ef) {

    string line;
    vector<string> line_splitted;
    int u, f;
    in_edge_iter ei, e_end;


    ifstream infile(feature_filename);
    // if (infile==NULL){
        if (! infile.is_open()){
        cout << "Unable to open the input file\n";
    }
    while(getline(infile, line)) {
        boost::split(line_splitted, line, boost::is_any_of(" "));
        u = stoi(line_splitted[0]);
        vector<int> u_features;
        for (int i=1; i < line_splitted.size(); ++i) {
            f = stoi(line_splitted[i]);
            u_features.push_back(f);
        }
        for (auto & feat: u_features) {
            for (boost::tie(ei, e_end) = in_edges(u, G); ei!=e_end; ++ei) {
                Ef[feat].push_back(make_pair(source(*ei, G), target(*ei, G)));
            }
        }
        Nf[u] = u_features;
    }
}

void read_probabilities(string prob_filename, edge_prob &P) {
    ifstream infile(prob_filename);
    // if (infile==NULL){
        if (! infile.is_open()){

        cout << "Unable to open the input file\n";
    }
    int u, v;
    double p;
    while (infile >> u >> v >> p) {
        P[make_pair(u, v)] = p;
    }
}

void read_probabilities2 (string prob_filename, vector<pair<int, int> > &order, vector<double> &P) {
    vector<vector<double> > edges;
    ifstream infile(prob_filename);
    // if (infile==NULL){
        if (! infile.is_open()){
        cout << "Unable to open the input file\n";
    }
    double u, v, p;

    while (infile >> u >> v >> p) {
        edges.push_back({u, v, p});
    }
    sort(edges.begin(), edges.end());

    for (auto &edge: edges) {
        order.push_back(make_pair((int) edge[0], (int) edge[1]));
        P.push_back(edge[2]);
    }
}

void read_groups(string group_filename, unordered_map<int, unordered_set<int> > &groups) {
    ifstream infile(group_filename);
    // if (infile==NULL){
        if (! infile.is_open()){
        cout << "Unable to open the input file\n";
    }
    string line;
    vector<string> line_splitted;

    while (getline(infile, line)) {
        boost::split(line_splitted, line, boost::is_any_of(" "));
        unordered_set<int> nodes;
        for (int i = 1; i < line_splitted.size(); ++i) {
            nodes.insert(stoi(line_splitted[i]));
        }
        groups[stoi(line_splitted[0])] = nodes;
    }
}

void read_seeds(string seeds_filename, unordered_set<int> &S, int length) {
    ifstream infile(seeds_filename);
    // if (infile==NULL){
        if (! infile.is_open()){
        cout << "Unable to open the input file\n";
    }
    int node, i=0;

    while (infile >> node and i < length) {
        S.insert(node);
        i++;
    }
}


edge_prob increase_probabilities(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > Nf, vector<int> F,
                                 vector<pair<int, int> > E, edge_prob &P) {
    edge_prob changed;
    double q,b,h;
    int target;
    double intersect;
    vector<int> F_target;
    for (auto &edge: E) {
        changed[edge] = P[edge];
        q = Q[edge]; b = B[edge];
        target = edge.second;
        F_target = Nf[target];
        sort(F_target.begin(), F_target.end());
        sort(F.begin(), F.end());
        unordered_set<int> s(F_target.begin(), F_target.end());
        intersect = count_if(F.begin(), F.end(), [&](int k) {return s.find(k) != s.end();});
        h = intersect/F_target.size();
        // cout<<"Change in edge weights: "<<P[edge]<<" :  ";
        P[edge] = h*q + b;
        // wait what about taking min with 1
        // cout<<P[edge]<<endl;
    }
    return changed;
}

void increase_prob_set(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > Nf, vector<int> F,
                                 unordered_map<int, vector<pair<int, int> > > Ef, edge_prob &P)
{
    
    P.clear(); // precaution
    P.insert(B.begin(), B.end());

    vector<pair<int, int> > E;
    for (int i =0; i<F.size(); ++i) {
        for (int j=0; j < Ef[F[i]].size(); ++j) {
            E.push_back(Ef[F[i]][j]); // basically all edges for the feature set
        }
    }

    increase_probabilities(G, B, Q, Nf, F, E, P); 
}


void decrease_probabilities(edge_prob changed, edge_prob &P) {
    for (auto &item: changed) {
        pair<int, int> edge = item.first;
        double p = item.second;
        P[edge] = p;
    }
}


double calculate_spread (DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > Nf, unordered_set<int> S,
                        vector<int> F, unordered_map<int, vector<pair<int, int> > > Ef, int I) {

    edge_prob Prob;
    Prob.insert(B.begin(), B.end());

    vector<pair<int, int> > E;
    for (int i =0; i<F.size(); ++i) {
        for (int j=0; j < Ef[F[i]].size(); ++j) {
            E.push_back(Ef[F[i]][j]);
        }
    }

    increase_probabilities(G, B, Q, Nf, F, E, Prob);

    double spread=0;
    pair<vertex_iter, vertex_iter> vp;
    unordered_map<int, bool> activated;
    vector<int> T;
    int u, v;
    double p;
    out_edge_iter ei, e_end;
    for (int it=0; it < I; ++it) {
        for (vp = boost::vertices(G); vp.first != vp.second; ++vp.first) {
            u = (int)*vp.first;
            activated[u] = false;
        }
        for (auto &node: S) {
            activated[node] = false;
            T.push_back(node);
        }
        int count = 0;
        while (count < T.size()) {
            u = T[count];
            for (boost::tie(ei, e_end) = out_edges(u, G); ei!=e_end; ++ei) {
                v = target(*ei, G);
                if (not activated[v]) {
                    p = Prob[make_pair(u, v)];
                    double r = ((double) rand() / (RAND_MAX));
                    if (r < p) {
                        activated[v] = true;
                        T.push_back(v);
                    }
                }
            }
            ++count;
        }
        spread += T.size();
        T.clear();
    }
    return spread/I;
}

int main(int argc, char const *argv[])
{
    // read all data
    unordered_map<int, vector<int> > Nf;
    unordered_map<int, vector<pair<int, int> > > Ef;
    edge_prob B, Q;// P;
    unordered_map<int, unordered_set<int> > groups;
    vector<int> F;
    unordered_set<int> S;
    int I, K, group_number;
    unordered_map<int, double> influence;
    double theta = 1./40, spread=0;
    int beam_width = 3;
    in_edge_iter qi, q_end;
    clock_t start, finish;
    string dataset_file, probs_file, features_file, groups_file, out_features_file, out_results_file, algo_name;
        // read parameters from command-line
    if (argc > 1) 
    {

        cout << "Got parameters..." << endl;
        string setup_file = argv[1];
        cout << setup_file << endl;

        //ifstream infile(setup_file);
        //if (infile==NULL){
            //cout << "Unable to open the input file\n";
        //}
        ifstream infile;
        infile.open(setup_file, ifstream::in);
        if (!infile.is_open()) 
        {
            cout << "Unable to open the input file in 'main' function\n";  exit(-1);
        }

        // Start input
        getline(infile, dataset_file);
        getline(infile, probs_file);
        getline(infile, features_file);
        getline(infile, groups_file);
        getline(infile, algo_name);

        string line;

        getline(infile, line);
        group_number = stoi(line);
        getline(infile, line);
        K = stoi(line);
        getline(infile, line);
        I = stoi(line);
        getline(infile, line);
        beam_width = stoi(line);
        cout << "Input:" << endl;
        cout << dataset_file << " " << probs_file << " " << features_file << " " << groups_file << endl;
        cout<<"group_number K I\n";
        cout << group_number << " " << K << " " << I << endl;
        cout<<"beam with: "<<beam_width<<endl;
        
    }
    else {
        cout << "Something went wrong! Exiting!" << endl;
        return 1;
    }

    DiGraph G = read_graph(dataset_file);
    read_features(features_file, G, Nf, Ef);
    read_probabilities(probs_file, B);
    read_probabilities(probs_file, Q);
    // read_probabilities(probs_file, P);
    read_groups(groups_file, groups);

    vector<int> Phi;
    for (auto &item: Ef) {
        Phi.push_back(item.first);
    }

    // SPECIFY SEEDS
    S = groups[group_number];
    // read_seeds(seeds_file, S, 15); // for VK network
    // S = groups[group_number]; // for Gnutella network
    cout << "S: ";
    for (auto &node: S) {
            cout << node << " ";
    }
    cout << endl;
    
    for (auto &node: S) {
        boost::clear_in_edges(node, G);
    }

    cout << "I: " << I << endl;
    cout << "K: " << K << endl;
    F = vector <int> ({1944, 60308, 704617, 13704358, 41199309});

    spread = calculate_spread(G, B, Q, Nf, S, F, Ef, I);
    cout<<"Final spread value: "<<spread<<endl;
    return 0;
}
