#include "helpers.h"

// Generic function, no change
void print_vertices(DiGraph G) {
    pair<vertex_iter, vertex_iter> vp;
    for (vp = boost::vertices(G); vp.first != vp.second; ++vp.first)
        cout << *vp.first << " " << *vp.second << endl;
    cout << endl;
}


// Generic function, no change
void print_edges(DiGraph G) {
    edge_iter ei, edge_end;
    for (boost::tie(ei, edge_end) = edges(G); ei != edge_end; ++ei) {
        cout << source(*ei, G) << " " << target(*ei, G) << endl;
    }
}

// See how to get in_degree for terminal vertex of an edge 
void print_degree(DiGraph G) {
    vertex_iter vi, v_end;
    int out_d, in_d, count=0;
    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi) {
        in_d = boost::in_degree(*vi, G); //***
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

    cout <<"graph file: "<<graph_filename << endl;
    //ifstream infile(graph_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(graph_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_graph' function\n";  exit(-1);
    }

    unordered_map<int, int> unordered_mapped;
    int u, v;
    int node_count=0;
    pair<DiGraph::edge_descriptor, bool> edge_insertion;
    DiGraph G;
    int lineno=0;
    while (infile >> u >> v) {
        lineno++;
        // cout << u << " "<< v <<endl;
        // cout<<"lineno "<<lineno<<endl;
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
    printf("graph read\n");
    return G;
}

DiGraph simple_read_graph(string graph_filename) {

    cout <<"graph file: "<<graph_filename << endl;
    //ifstream infile(graph_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(graph_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_graph' function\n";  exit(-1);
    }

    unordered_map<int, int> unordered_mapped;
    int u, v;
    int node_count=0;
    pair<DiGraph::edge_descriptor, bool> edge_insertion;
    DiGraph G;
    int lineno=0;
    while (infile >> u >> v) {
        lineno++;
        // cout << u << " "<< v <<endl;
        // cout<<"lineno "<<lineno<<endl;
        edge_insertion=boost::add_edge(u, v, G);
        if (!edge_insertion.second) {
            std::cout << "Unable to insert edge\n";
        }
    }
    printf("graph read\n");
    return G;
}


void read_features(string feature_filename, DiGraph G, unordered_map<int, vector<int> > &Nf, unordered_map<int, vector<pair<int, int> > > &Ef) {

    string line;
    vector<string> line_splitted;
    int u, f;
    in_edge_iter ei, e_end;

    //ifstream infile(feature_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(feature_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_features' function\n";  exit(-1);
    }

    while(getline(infile, line)) {
        boost::split(line_splitted, line, boost::is_any_of(" "));
        u = stoi(line_splitted[0]);
        vector<int> u_features;
        for (unsigned i=1; i < line_splitted.size(); ++i) {
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
    //ifstream infile(prob_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(prob_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_probabilities' function\n";  exit(-1);
    }

    int u, v;
    double p;
    while (infile >> u >> v >> p) {
        P[make_pair(u, v)] = p;
    }
}

void read_probabilities2(string prob_filename, vector<pair<int, int> > &order, vector<double> &P) {
    //ifstream infile(prob_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(prob_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_probabilities2' function\n";  exit(-1);
    }

    vector<vector<double> > edges;
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
    //ifstream infile(group_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(group_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_groups' function\n";  exit(-1);
    }

    string line;
    vector<string> line_splitted;

    while (getline(infile, line)) {
        boost::split(line_splitted, line, boost::is_any_of(" "));
        unordered_set<int> nodes;
        for (unsigned i = 1; i < line_splitted.size(); ++i) {
            nodes.insert(stoi(line_splitted[i]));
        }
        groups[stoi(line_splitted[0])] = nodes;
    }
}

void read_seeds(string seeds_filename, unordered_set<int> &S, int length) {
    //ifstream infile(seeds_filename);
    //if (infile==NULL){
        //cout << "Unable to open the input file\n";
    //}
    ifstream infile;
    infile.open(seeds_filename, ifstream::in);
    if (!infile.is_open()) {
        cout << "Unable to open the input file in 'read_seeds' function\n";  exit(-1);
    }

    int node, i=0;

    while (infile >> node and i < length) {
        S.insert(node);
        i++;
    }
}

void increase_prob_set(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F,
                                 unordered_map<int, vector<pair<int, int> > > feat_to_edges, edge_prob &P, unordered_map <int, double> &in_degrees)
{
    
    vector<pair<int, int> > E;
    for (int i =0; i<F.size(); ++i) {
        for (int j=0; j < feat_to_edges[F[i]].size(); ++j) {
            E.push_back(feat_to_edges[F[i]][j]); // basically all edges for the feature set
        }
    }

    increase_probabilities(G, B, Q, node_to_feat, F, E, P, in_degrees); 
}


edge_prob increase_probabilities(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F,
                                 vector<pair<int, int> > E, edge_prob &P, unordered_map <int, double> &in_degrees) 
{
    // specify E as an argument to speed up 
    edge_prob changed;
    double q, b, h;
    int target;
    double intersection;
    vector <int> F_target;
    int edge_count=0, inc_count=0;

    for (auto &edge: E) {
        changed[edge] = P[edge]; // store old value
        // cout<<"old value: "<<P[edge]<< " ";
        q = Q[edge]; b = B[edge];
        target = edge.second;
        F_target = node_to_feat[target]; // Fv
        sort(F_target.begin(), F_target.end());
        sort(F.begin(), F.end());
        unordered_set<int> s(F_target.begin(), F_target.end());
        intersection = count_if(F.begin(), F.end(), [&](int k) {return s.find(k) != s.end();});
        // assume that set intersection method is implemented correctly in Explore-Update
        h = intersection; // intersection only. Q is calculated approp
        P[edge] = (double) (1./in_degrees[target]) * sigmoid(h*q + b);
        edge_count++;
        if (P[edge] - changed[edge] > 0)
        {
            inc_count++;
        }
        // cout<<"b: "<<b<<"q: "<<q<<"h: "<<h<<" ";
        // cout <<"New value: "<<P[edge]<<endl;
    }
    // cout<<inc_count<<" out of "<<edge_count<<" edges had an increase in weight\n";
    return changed;
}

edge_prob increase_probabilities_no_transform(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F,
                                 vector<pair<int, int> > E, edge_prob &P, unordered_map <int, double> &in_degrees) 
{
    // specify E as an argument to speed up 
    edge_prob changed;
    double q, b, h;
    int target;
    double intersection;
    vector <int> F_target;
    int edge_count=0, inc_count=0;

    for (auto &edge: E) {
        changed[edge] = P[edge]; // store old value
        // cout<<"old value: "<<P[edge]<< " ";
        q = Q[edge]; b = B[edge];
        target = edge.second;
        F_target = node_to_feat[target]; // Fv
        sort(F_target.begin(), F_target.end());
        sort(F.begin(), F.end());
        unordered_set<int> s(F_target.begin(), F_target.end());
        intersection = count_if(F.begin(), F.end(), [&](int k) {return s.find(k) != s.end();});
        // assume that set intersection method is implemented correctly in Explore-Update
        h = intersection; // intersection only. Q is calculated approp
        
        P[edge] =  (double)  (h*q + b > 1.0/in_degrees[target])? (1./in_degrees[target]) : h*q + b;
        edge_count++;
        if (P[edge] - changed[edge] > 0)
        {
            inc_count++;
        }
        // cout<<"b: "<<b<<"q: "<<q<<"h: "<<h<<" ";
        // cout <<"New value: "<<P[edge]<<endl;
    }
    // cout<<inc_count<<" out of "<<edge_count<<" edges had an increase in weight\n";
    return changed;
}

edge_prob init_probs(DiGraph G, edge_prob Btransformed, unordered_map <int, double> &in_degrees)
{
    edge_iter ei, edge_end;
    pair <int, int> edge;
    int v;
    edge_prob P;
    for (boost::tie(ei, edge_end) = edges(G); ei != edge_end; ++ei) 
    {
        
        // cout << source(*ei, G) << " " << target(*ei, G) << endl;
        edge = make_pair(source(*ei, G), target(*ei, G));
        v = edge.second;
        double d_in = in_degrees[v];
        
        P[edge] = 1.0/d_in * sigmoid(Btransformed[edge]);
        
    }
    
    return P;
}

edge_prob init_probs_no_transform(DiGraph G, edge_prob B, unordered_map <int, double> &in_degrees)
{
    edge_iter ei, edge_end;
    pair <int, int> edge;
    int v;
    edge_prob P;
    for (boost::tie(ei, edge_end) = edges(G); ei != edge_end; ++ei) 
    {
        
        // cout << source(*ei, G) << " " << target(*ei, G) << endl;
        edge = make_pair(source(*ei, G), target(*ei, G));
        v = edge.second;
        double d_in = in_degrees[v];
        if (1.0/d_in < B[edge])
        {
            P[edge] = 1.0/d_in;
        }
        else
        {
            P[edge] = B[edge];
        }
    }
    
    return P;
}


// copy-paste
void decrease_probabilities(edge_prob changed, edge_prob &P) {
    for (auto &item: changed) {
        pair<int, int> edge = item.first;
        double p = item.second;
        P[edge] = p;
    }
}


unordered_map<int, double> save_in_degrees(DiGraph G)
{
    unordered_map<int, double> in_degrees;
    vertex_iter vi, v_end;
    int in_d;
    // FILE* deg_file;// - can load instead of recomputing
    // deg_file = fopen("degrees.txt", "w"); 

    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi)
    {
        in_d = boost::in_degree(*vi, G);
        in_degrees.insert(make_pair(*vi, in_d));
        // fprintf(deg_file, "%d %d\n", *vi, in_d);
    }
    // fclose(deg_file);
    return in_degrees;
}

unordered_map<int, double> save_out_degrees(DiGraph G)
{
    unordered_map<int, double> out_degrees;
    vertex_iter vi, v_end;
    int out_d;
    // FILE* deg_file;// - can load instead of recomputing
    // deg_file = fopen("out_degrees.txt", "w"); 

    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi)
    {
        out_d = boost::out_degree(*vi, G);
        out_degrees.insert(make_pair(*vi, out_d));
        // fprintf(deg_file, "%d %d\n", *vi, out_d);
    }
    // fclose(deg_file);
    return out_degrees;
}


double sigmoid(double x) // scalar. No need to vectorize?
{
    return (double) 1./(1.0 + exp(-x));
}
