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

void print_vertices(DiGraph G) {
    pair<vertex_iter, vertex_iter> vp;
    for (vp = boost::vertices(G); vp.first != vp.second; ++vp.first)
        cout << *vp.first << " " << *vp.second << endl;
    cout << endl;
}

void print_edges(DiGraph G) {
    edge_iter ei, edge_end;
    for (boost::tie(ei, edge_end) = edges(G); ei != edge_end; ++ei) {
        cout << source(*ei, G) << " " << target(*ei, G) << endl;
    }
}

void print_degree(DiGraph G) {
    vertex_iter vi, v_end;
    int out_d, in_d, count=0;
    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi) {
        in_d = boost::in_degree(*vi, G);
        out_d = boost::out_degree(*vi, G);
        cout << *vi << " " << out_d << " " << in_d << endl;
    }
}

void print_node_edges(DiGraph G) {
    out_edge_iter ei, e_end;
    in_edge_iter qi, q_end;
    vertex_iter vi, v_end;
    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi) {
        cout << *vi << "--->";
        for (boost::tie(ei, e_end) = out_edges(*vi, G); ei!=e_end; ++ei) {
            cout << target(*ei, G) << " ";
        }
        cout << endl;
        cout << *vi << "<---";
        for (boost::tie(qi, q_end) = in_edges(*vi, G); qi!=q_end; ++qi) {
            cout << source(*qi, G) << " ";
        }
        cout << endl;
        cout << endl;
    }
}

void print_size(DiGraph G) {
    cout << num_vertices(G) << endl;
    cout << num_edges(G) << endl;
}

DiGraph read_graph(string graph_filename) {
    cout << graph_filename << endl;
    ifstream infile(graph_filename);
    if (infile==NULL){
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
    if (infile==NULL){
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
    if (infile==NULL){
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
    if (infile==NULL){
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
    if (infile==NULL){
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
    if (infile==NULL){
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
        P[edge] = h*q + b;
    }
    return changed;
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

pair<vector<int>, unordered_map<int, double> >  greedy(DiGraph G, edge_prob B, edge_prob Q, unordered_set<int> S, unordered_map<int,
        vector<int> > Nf, unordered_map<int, vector<pair<int, int> > > Ef, vector<int> Phi, int K, int I) {

    vector<int> F;
    edge_prob P;
    unordered_map<int, bool> selected;
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    unordered_map<int, double> influence;

    P.insert(B.begin(), B.end());

    while (F.size() < K) {
        max_spread = -1;
        printf("it = %i; ", (int)F.size() + 1);
        fflush(stdout);
        for (auto &f: Phi) {
            cout << f << " ";
            fflush(stdout);
            if (not selected[f]) {
                F.push_back(f);
                changed = increase_probabilities(G, B, Q, Nf, F, Ef[f], P);
                spread = calculate_spread(G, B, Q, Nf, S, F, Ef, I);
                if (spread > max_spread) {
                    max_spread = spread;
                    max_feature = f;
                }
                decrease_probabilities(changed, P);
                F.pop_back();
            }
        }
        F.push_back(max_feature);
        selected[max_feature] = true;
        printf("f = %i; spread = %.2f\n", max_feature, max_spread);
        increase_probabilities(G, B, Q, Nf, F, Ef[max_feature], P);
        influence[F.size()] = max_spread;
    }
    return make_pair(F, influence);
}

unordered_map<int, set<pair<int, int> > > explore(DiGraph G, edge_prob P, unordered_set<int> S, double theta) {

    double max_num = numeric_limits<double>::max();
    double min_dist;
    pair<int, int> min_edge, mip_edge;
    pair<DiGraph::edge_descriptor, bool> edge_insertion;
    int V = num_vertices(G);
    map<pair<int, int>, double> edge_weights;
    out_edge_iter ei, e_end;
    in_edge_iter qi, q_end;
    unordered_map<int, double> dist;
    set<pair<int, int> > crossing_edges;
    unordered_map<int, vector<pair<int, int> > > MIPs;
    unordered_map<int, set<pair<int, int> > > Ain_edges;


    for (auto &v: S) {
        MIPs[v] = {};
        dist[v] = 0;
        for (boost::tie(ei, e_end) = out_edges(v, G); ei!=e_end; ++ei) {
            crossing_edges.insert(make_pair(source(*ei, G), target(*ei, G)));
        }

        while (true) {
            if (crossing_edges.size() == 0)
                break;

            min_dist = max_num;
            min_edge = make_pair(V+1, V+1);

            for (auto &edge: crossing_edges) {
                if (edge_weights.find(edge) == edge_weights.end()) {
                    edge_weights[edge] = -log(P[edge]);
                }
                if (edge_weights[edge] + dist[edge.first] < min_dist or
                        (edge_weights[edge] + dist[edge.first] == min_dist and edge <= min_edge)) {
                    min_dist = edge_weights[edge] + dist[edge.first];
                    min_edge = edge;
                }
            }
            if (min_dist <= -log(theta)) {
                dist[min_edge.second] = min_dist;
                MIPs[min_edge.second] = MIPs[min_edge.first];
                MIPs[min_edge.second].push_back(min_edge);
                for (auto &edge: MIPs[min_edge.second]) {
                    Ain_edges[min_edge.second].insert(edge);
                }

                for (boost::tie(qi, q_end) = in_edges(min_edge.second, G); qi!=q_end; ++qi) {
                    crossing_edges.erase(make_pair(source(*qi, G), target(*qi, G)));
                }
                for (boost::tie(ei, e_end) = out_edges(min_edge.second, G); ei!=e_end; ++ei) {
                    int end2 = target(*ei, G);
                    if (MIPs.find(end2) == MIPs.end()) {
                        crossing_edges.insert(make_pair(min_edge.second, end2));
                    }
                }
            }
            else
                break;
        }
        dist.clear();
        crossing_edges.clear();
        MIPs.clear();
    }
    return Ain_edges;
}

SubGraph make_subgraph(set<pair<int, int> > Ain_edges_v, int root) {
    SubGraph Ain_v;
    int u, v, count=0;
    unordered_map<int, int> unordered_mapped;
    edge_t e; bool b;
    vertex_t vertex;

    unordered_mapped[root] = count;
    vertex = boost::add_vertex(Ain_v);
    Ain_v[vertex].label = root;
    count++;
    for (auto &edge: Ain_edges_v) {
        u = edge.first; v = edge.second;
        if (unordered_mapped.find(u) == unordered_mapped.end()) {
            unordered_mapped[u] = count;
            vertex = boost::add_vertex(Ain_v);
            Ain_v[vertex].label = u;
            count++;
        }
        if (unordered_mapped.find(v) == unordered_mapped.end()) {
            unordered_mapped[v] = count;
            vertex = boost::add_vertex(Ain_v);
            Ain_v[vertex].label = v;
            count++;
        }
        boost::tie(e, b) = boost::add_edge(unordered_mapped[u], unordered_mapped[v], Ain_v);
        if (not b)
            cout << "Unable to insert an edge in Ain_v" << endl;
    }
    return Ain_v;
}

double calculate_ap(vertex_t u, SubGraph Ain_v, unordered_set<int> S, edge_prob P) {
    if (S.find(Ain_v[u].label) != S.end())
        return 1;
    else {
        double prod = 1, ap_node, p;
        in_edge_iter qi, q_end;
        vertex_t node;
        clock_t start, finish;
        for (boost::tie(qi, q_end)=in_edges(u, Ain_v); qi!=q_end; ++qi) {
            node = source(*qi, Ain_v);
            ap_node = calculate_ap(node, Ain_v, S, P);
            p = P[make_pair(Ain_v[node].label, Ain_v[u].label)];
            prod *= (1 - ap_node*p);
        }
        return 1 - prod;
    }
}

double calculate_ap2(SubGraph Ain_v, unordered_set<int> S, edge_prob P) {
    vector<vertex_t> topology;
    unordered_map<vertex_t, double> ap;
    double prod;
    in_edge_iter qi, q_end;

    topological_sort(Ain_v, back_inserter(topology));

    clock_t start = clock();
    for (vector<vertex_t>::reverse_iterator ii=topology.rbegin(); ii!=topology.rend(); ++ii) {
        if (S.find(Ain_v[*ii].label) != S.end()) {
            ap[*ii] = 1;
        }
        else {
            prod = 1;
            for (boost::tie(qi, q_end)=in_edges(*ii, Ain_v); qi!=q_end; ++qi) {
                prod *= (1 - ap[source(*qi, Ain_v)]*P[make_pair(Ain_v[source(*qi, Ain_v)].label, Ain_v[*ii].label)]);
            }
            ap[*ii] = 1 - prod;
        }
    }
    return 1 - prod;
}

double calculate_ap3(set<pair<int, int> >& Ain_edges_v, unordered_set<int> S, edge_prob P, int node, unordered_map<int, double> & ap_values) {
    if (S.find(node) != S.end())
        return 1;
    else {
        double prod = 1, ap_node;

        for (auto &edge: Ain_edges_v) {
            if (edge.second == node) {
                if (ap_values.find(edge.first) != ap_values.end()) {
                    ap_node = ap_values[edge.first];
                }
                else {
                    ap_node = calculate_ap3(Ain_edges_v, S, P, edge.first, ap_values);
                }
                prod *= (1 - ap_node*P[edge]);
                Ain_edges_v.erase(edge);
            }
        }
        ap_values[node] = 1 - prod;
        return 1 - prod;
    }
}

double update(unordered_map<int, set<pair<int, int> > > Ain_edges, unordered_set<int> S, edge_prob P) {
    double total = 0, count=0, path_prob;
    unordered_set<int> mip;
    bool pathed;
    unordered_map <int, double> ap_values;
    for (auto &item: Ain_edges) {
        pathed = true;
        path_prob = 1;
        set<pair<int, int> > edges = item.second;
        for (const auto &e: edges) {
            if (mip.find(e.second) != mip.end()) {
                pathed = false;
                break;
            }
            else {
                mip.insert(e.second);
                path_prob *= P[e];
            }
        }
        if (pathed) {
            count++;
            total += path_prob;
        }
        else {
            SubGraph Ain_v = make_subgraph(Ain_edges[item.first], item.first);
            total += calculate_ap2(Ain_v, S, P);
        }
    }
    return total;
}

set<pair<int, int> > get_pi(DiGraph G, unordered_map<int, set<pair<int, int> > > Ain_edges, unordered_set<int> S) {
    set<pair<int, int> > Pi;
    out_edge_iter ei, e_end;
    in_edge_iter qi, q_end;
    vertex_iter vi, v_end;
    set<int> Pi_nodes;

    Pi_nodes.insert(S.begin(), S.end());
    for (auto &item: Ain_edges) {
        Pi_nodes.insert(item.first);
    }

    for (auto &node: Pi_nodes) {
        for (boost::tie(ei, e_end) = out_edges(node, G); ei!=e_end; ++ei) {
            Pi.insert(make_pair(source(*ei, G), target(*ei, G)));
        }
        for (boost::tie(qi, q_end) = in_edges(node, G); qi!=q_end; ++qi) {
            Pi.insert(make_pair(source(*qi, G), target(*qi, G)));
        }
    }
    return Pi;
}

vector<int> explore_update(DiGraph G, edge_prob B, edge_prob Q, edge_prob P, unordered_set<int> S, unordered_map<int,vector<int> > Nf,
                           unordered_map<int, vector<pair<int, int> > > Ef, vector<int> Phi, int K, double theta) {


    vector<int> F;
    unordered_map<int, set<pair<int, int> > > Ain_edges;
    set<pair<int, int> > Pi;
    int max_feature;
    double max_spread, spread;
    unordered_map<int, bool> selected;
    bool intersected;
    edge_prob changed;
    int omissions = 0;
    clock_t begin, finish;

    cout << "Starting Explore-Update.\nInitializing..." << endl;
    Ain_edges = explore(G, P, S, theta);
    Pi = get_pi(G, Ain_edges, S);
    cout << "Finished initializiation.\nStart selecting features..." << endl;

    while (F.size() < K) {
        cout << F.size() << ": ";
        fflush(stdout);
        max_feature = -1;
        max_spread = -1;
        int count = 0;
        begin = clock();
        for (auto &f: Phi) {
            if (count%100 == 0) {
                cout << count << " ";
                fflush(stdout);
            }
            count++;
            if (not selected[f]) {
                intersected = false;
                for (auto &edge: Ef[f]) {
                    if (Pi.find(edge) != Pi.end()) {
                        intersected = true;
                        break;
                    }
                }
                if (intersected) {
                    F.push_back(f);
                    changed = increase_probabilities(G, B, Q, Nf, F, Ef[f], P);
                    Ain_edges = explore(G, P, S, theta);
                    spread = update(Ain_edges, S, P);
                    if (spread > max_spread) {
                        max_spread = spread;
                        max_feature = f;
                    }
                    decrease_probabilities(changed, P);
                    F.pop_back();
                }
                else {
                    ++omissions;
                }
            }
        }
        finish = clock();
        cout << (double) (finish - begin)/CLOCKS_PER_SEC;
        cout << endl;
        F.push_back(max_feature);
        selected[max_feature] = true;
        increase_probabilities(G, B, Q, Nf, F, Ef[max_feature], P);
    }
    cout << "Total number of omissions: " << omissions << endl;
    return F;
}

vector<int> top_edges(unordered_map<int, vector<pair<int, int> > > Ef, int K) {

    vector<pair<int, int> > tuples;
    int len;
    for (auto &item: Ef) {
        len = item.second.size();
        tuples.push_back(make_pair(item.first, len));
    }
    sort(tuples.begin(), tuples.end(), [](const pair<int,int> &left, const pair<int,int> &right) {
        return left.second > right.second;
    });
    vector<int> F;
    for (auto &t: tuples) {
        F.push_back(t.first);
        if (F.size() == K)
            return F;
    }
}

vector<int> top_nodes(unordered_map<int, vector<int> > Nf, int K) {
    vector<int> F;
    unordered_map<int, int> degrees;
    vector<int> nodes;vector<pair<int, int> > tuples;
    int len;

    for (auto &item: Nf) {
        for (int i=0; i<item.second.size(); ++i) {
            ++degrees[item.second[i]];
        }
    }
    for (auto &item: degrees) {
        tuples.push_back(item);
    }
    sort(tuples.begin(), tuples.end(), [](const pair<int,int> &left, const pair<int,int> &right) {
        return left.second > right.second;
    });
    for (auto &t: tuples) {
        F.push_back(t.first);
        if (F.size() == K)
            return F;
    }
}

double test(SubGraph Ain_v, edge_prob P, unordered_set<int> S) {
    vector<vertex_t> topology;
    unordered_map<vertex_t, double> ap;
    double prod=1, p, active_p;
    in_edge_iter qi, q_end;
    pair<int, int> e;
    clock_t start, finish;

    topological_sort(Ain_v, back_inserter(topology));

    for (vector<vertex_t>::reverse_iterator ii=topology.rbegin(); ii!=topology.rend(); ++ii) {
        if (S.find(Ain_v[*ii].label) != S.end()) {
            ap[*ii] = 1;
        }
        else {
            for (boost::tie(qi, q_end) = in_edges(*ii, Ain_v); qi != q_end; ++qi) {
                start = clock();
                e = make_pair(Ain_v[source(*qi, Ain_v)].label, Ain_v[*ii].label);
                finish = clock();
                start = clock();
                p = P[e];
                finish = clock();
                start = clock();
                active_p = ap[source(*qi, Ain_v)];
                finish = clock();
                prod *= (1 - active_p*p);
            }
            ap[*ii] = 1 - prod;
        }
    }
    return 1 - prod;
}

int main(int argc, char* argv[]) {
//    srand(time(NULL));

    unordered_map<int, vector<int> > Nf;
    unordered_map<int, vector<pair<int, int> > > Ef;
    edge_prob B, Q, P;
    unordered_map<int, unordered_set<int> > groups;
    vector<int> F;
    unordered_set<int> S;
    int I, K, group_number;
    unordered_map<int, double> influence;
    double theta = 1./40, spread=0;
    in_edge_iter qi, q_end;
    clock_t start, finish;
    string dataset_file, probs_file, features_file, groups_file, out_features_file, out_results_file, seeds_file;

    // read parameters from command-line
    if (argc > 1) {

        cout << "Got parameters..." << endl;
        string setup_file = argv[1];
        cout << setup_file << endl;
       ifstream infile(setup_file);
       if (infile==NULL){
           cout << "Unable to open the input file\n";
       }

       getline(infile, dataset_file);
       getline(infile, probs_file);
       getline(infile, features_file);
       getline(infile, groups_file);
       getline(infile, seeds_file);

       string line;
       getline(infile, line);
       group_number = stoi(line);
       getline(infile, line);
       K = stoi(line);
       getline(infile, line);
       I = stoi(line);

       cout << "Input:" << endl;
       cout << dataset_file << " " << probs_file << " " << features_file << " " << groups_file << endl;
       cout << group_number << " " << K << " " << I << endl;

    }
    else {
        cout << "Something went wrong! Exiting!" << endl;
        return 1;
    }

   DiGraph G = read_graph(dataset_file);
   read_features(features_file, G, Nf, Ef);
   read_probabilities(probs_file, B);
   read_probabilities(probs_file, Q);
   read_probabilities(probs_file, P);
   read_groups(groups_file, groups);

   vector<int> Phi;
   for (auto &item: Ef) {
       Phi.push_back(item.first);
   }

   // SPECIFY SEEDS
   read_seeds(seeds_file, S, 15); // for VK network
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
   FILE *results_f, *outfile;

  cout << "Start greedy algorithm..." << endl;
  start = clock();
  boost::tie(F, influence) = greedy(G, B, Q, S, Nf, Ef, Phi, K, I);
  finish = clock();
//    writing selected features, time, and spread
  outfile = fopen("greedy_features.txt", "w"); // SPECIFY OUTPUT FILE FOR FEATURES
  cout << "Features: ";
  for (auto &f: F) {
      fprintf(outfile, "%i ", f);
      cout << f << " ";
  }
  fclose(outfile);
  cout << endl;
  results_f = fopen("greedy_results.txt", "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
  fprintf(results_f, "%f\n", (double) (finish - start)/CLOCKS_PER_SEC);
  cout << (double) (finish - start)/CLOCKS_PER_SEC << " sec." << endl;
  for (int num = 1; num <= K; ++num) {
      fprintf(results_f, "%f\n", influence[num]);
      cout << num << ": " << influence[num]  << " spread" << endl;
  }
  fclose(results_f);
  cout << endl;

   //    top-edges heuristic
  cout << "Start Top-Edges..." << endl;
  F.clear();
  start = clock();
  F = top_edges(Ef, K);
  finish = clock();

  results_f = fopen("tope_results.txt", "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
  fprintf(results_f, "%f\n", (double) (finish - start)/CLOCKS_PER_SEC);
  cout << (double) (finish - start)/CLOCKS_PER_SEC << " sec." << endl;
  for (int num = 1; num <= K; num+=5) {
      vector<int> subv(F.begin(), F.begin()+num);
      spread = calculate_spread(G, B, Q, Nf, S, subv, Ef, I);
      fprintf(results_f, "%f\n", spread);
      cout << num << ": " << spread << endl;
  }
  fclose(results_f);
   cout << endl;

   sort(F.begin(), F.end());
  outfile = fopen("tope_features.txt", "w"); // SPECIFY OUTPUT FILE FOR FEATURES
  cout << "Features: ";
  for (auto &f: F) {
      fprintf(outfile, "%i ", f);
      cout << f << " ";
  }
  fclose(outfile);
  cout << endl;

    //    top-nodes heuristic
  cout << "Start Top-Nodes..." << endl;
  F.clear();
  start = clock();
  F = top_nodes(Nf, K);
  finish = clock();

  results_f = fopen("topn_results.txt", "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
  fprintf(results_f, "%f\n", (double) (finish - start)/CLOCKS_PER_SEC);
  cout << (double) (finish - start)/CLOCKS_PER_SEC << " sec." << endl;
  for (int num = 1; num <= K; num+=5) {
      vector<int> subv(F.begin(), F.begin()+num);
      spread = calculate_spread(G, B, Q, Nf, S, subv, Ef, I);
      fprintf(results_f, "%f\n", spread);
      cout << num << ": " << spread << endl;
  }
  fclose(results_f);
  cout << endl;

  sort(F.begin(), F.end());
  outfile = fopen("topn_features.txt", "w"); // SPECIFY OUTPUT FILE FOR FEATURES
  cout << "Features: ";
  for (auto &f: F) {
      fprintf(outfile, "%i ", f);
      cout << f << " ";
  }
  fclose(outfile);
  cout << endl;

   // Explore-Update algorithm
   // if features are stored in the file (copy this, instead of calculating features, for other heuristics too).
   // string filename;
   // ifstream infile;
   // int f;
   // filename = "results/experiment1_10000/mv/gnutella_eu_features.txt";
   // infile.open(filename);
   // if (infile == NULL)
   //     cout << "Cannot open file" << endl;
   // while (infile >> f) {
   //     F.push_back(f);
   // }
   // infile.close();
//    if features should be calculated here
   cout << "Start explore-update" << endl;
   F.clear();
   start = clock();
   F = explore_update(G, B, Q, P, S, Nf, Ef, Phi, K, theta);
   finish = clock();

   results_f = fopen("eu_results.txt", "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
   fprintf(results_f, "%f\n", (double) (finish - start)/CLOCKS_PER_SEC);
   cout << (double) (finish - start)/CLOCKS_PER_SEC << " sec." << endl;
   for (int num = 1; num <= K; num+=5) {
       vector<int> subv(F.begin(), F.begin()+num);
       spread = calculate_spread(G, B, Q, Nf, S, subv, Ef, I);
       fprintf(results_f, "%f\n", spread);
       cout << num << ": " << spread << endl;
   }
   fclose(results_f);
   cout << endl;

   
   sort(F.begin(), F.end());
   outfile = fopen("eu_features.txt", "w"); // SPECIFY OUTPUT FILE FOR FEATURES
   cout << "Features: ";
   for (auto &f: F) {
       fprintf(outfile, "%i ", f);
       cout << f << " ";
   }
   fclose(outfile);
   cout << endl;

   return 0;
}