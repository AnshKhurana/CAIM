#include "helpers.h"
#define inf 100 
// for simgoid sgm(100) ~ 1
// borrow print and read functions from explore-update
// edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B)
edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B, unordered_map<int, double> &in_degrees);
void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q, unordered_map<int, double> &in_degrees); // or B?
double calculate_MC_spread(DiGraph G, edge_prob P, unordered_map <int, vector<int> > node_to_feat);
pair<vector<int>, unordered_map<int, double> >  greedy(DiGraph G, edge_prob base, edge_prob Btransformed, edge_prob Q, unordered_set<int> S, unordered_map<int, vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> Phi, int K, int I, unordered_map <int, double> &in_degrees);
edge_prob increase_probabilities(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F, vector<pair<int, int> > E, edge_prob &P, unordered_map <int, double> &in_degrees);
void decrease_probabilities(edge_prob changed, edge_prob &P);
edge_prob init_probs(DiGraph, edge_prob, unordered_map <int, double> &in_degrees);

int main(int argc, char const *argv[])
{
    
    // Declarations
    unordered_map <int, vector<int> > node_to_feat;
    unordered_map<int, vector<pair<int, int> > > feat_to_edges;
    edge_prob B, Q;
    unordered_map<int, unordered_set<int> > groups; // feat to nodes
    unordered_set<int> S; // Seed set
    int I, K, group_number; 
    clock_t start, finish;
    double final_spread;
    vector <int> result_feature_set;
    unordered_map<int, double> influence; // for greedy algo
    unordered_map <int, double> in_degrees; // double to prevent divsion type casts
    string dataset_file, b_file, q_file, mem_file, algo_name, groups_file,
     seeds_file, save_dir, save_result, save_features, out_features_file, out_results_file;

    if (argc > 1)
    {
        cout<<"Reading parameters...\n";
        string setup_file = argv[1];
        cout<<"Setup file: "<<setup_file<<endl;
        ifstream infile;
        infile.open(setup_file, ifstream::in);
        if (!infile.is_open()) {
            cout << "Unable to open the setup file in 'main' function\n";  exit(-1);
        }
        getline(infile, dataset_file);
        getline(infile, b_file);
        getline(infile, q_file);
        getline(infile, mem_file);
        getline(infile, groups_file); //com
        getline(infile, seeds_file);
        getline(infile, algo_name); 

        string line;

        getline(infile, line);
        group_number = stoi(line);
        getline(infile, line);
        K = stoi(line);
        getline(infile, line);
        I = stoi(line);

        // Print
        cout << "dataset_file" << " " << "b_file" << " "  << "q_file" << " " <<
                "mem_file" << " " << "groups_file" << endl;
        
        cout << dataset_file << " " << b_file << " " << q_file << " " <<
                mem_file << " " << groups_file << endl;
        
        cout<<"group_number K I\n";
        cout << group_number << " " << K << " " << I << endl;

        save_dir = "./results/" + algo_name + "/" + setup_file + "/";
        cout<<"save dir: "<<save_dir<<endl;

        save_result = save_dir + "results.txt";
        save_features = save_dir + "features.txt";
    }
    else
    {
        cout<<"Missing args file\n";
        exit(1);
    }


    // Read files

    DiGraph G = read_graph(dataset_file);
    read_features(mem_file, G, node_to_feat, feat_to_edges);
    read_probabilities(b_file, B);
    in_degrees = save_degrees(G);
    edge_prob Bt =  transform_probabilties(node_to_feat, B, in_degrees);
    
    if (q_file == "-")
    {
        gen_q(B, node_to_feat, Q, in_degrees); // or B?
    }
    else
    {
        read_probabilities(q_file, Q);
    }

    read_groups(groups_file, groups);

    // Phi - set of all features
    vector<int> Phi;
    for (auto &item: feat_to_edges) 
    {
        Phi.push_back(item.first);
    }

    if (seeds_file == "-")
    {
        S = groups[group_number];
    }
    else
    {
        read_seeds(seeds_file, S, 20); // only for VK network
    }

    cout << "S: ";

    for (auto &node: S) 
    {
        cout << node << " ";
    }

    cout << endl;

    // One cannot activate a seed node, remove incoming edges
    for (auto &node: S)
    {
        boost::clear_in_edges(node, G); // # smart move
    }

    cout << "I: " << I << endl;
    cout << "K: " << K << endl;
    FILE *results_file; // variables for files


    // Time to estimate the spread
    cout << "Starting algo: " <<algo_name<< endl;
    start = clock();

    // sets up result_feature_set
    if (strcmp(algo_name.c_str(), "simpath") == 0)
    {
        //...
    }
    else if (strcmp(algo_name.c_str(), "rr") == 0)
    {
        
        //...
        
    }
    else
    {
        boost::tie(result_feature_set, influence) = greedy(G, B, Bt, Q, S, node_to_feat, feat_to_edges, Phi, K, I, in_degrees);
    }
    finish = clock();
    cout<<"No seg fault yet!0\n";
    results_file = fopen(save_result.c_str(), "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
    double exec_time = (double) (finish - start)/CLOCKS_PER_SEC;
    cout<<"No seg fault yet!1\n";
    fprintf(results_file, "Execution time  = %f\n secs.", exec_time);
    cout << "Execution time = " << exec_time << " secs.";
    
    // final_spread = calculate_MC_spread(result_feature_set); // ... for LTM 

    for (int num = 1; num <= K; ++num) {
      fprintf(results_file, "%f\n", influence[num]);
      cout << num << ": " << influence[num]  << " spread" << endl;
    }
    final_spread = influence[K];
    cout<<"Final spread value: "<<final_spread<<endl;

    fprintf(results_file, "The final spread value = %f\n", final_spread);
    fclose(results_file);

    sort(result_feature_set.begin(), result_feature_set.end());
    results_file = fopen(save_features.c_str(), "w"); // SPECIFY OUTPUT FILE FOR FEATURES
    cout << "Features: ";
    for (auto &f: result_feature_set) {
        fprintf(results_file, "%i ", f);
        cout << f << " ";
    }
    fclose(results_file);
    cout << endl;
    return 0;
}



edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B, unordered_map<int, double> &in_degrees)
{
    edge_prob Btransformed;
    double b; int v;
    double d_in;
    pair <int, int> edge;
    int overall_count=0, high_count=0;
    for (auto &it : B)
    {
        b = it.second;
        edge = it.first;
        v = edge.second;
        d_in = in_degrees[v];
        if (1-b*d_in < 0)
        {
            Btransformed[edge] = inf;
            high_count++;
            overall_count++;
        }
        else
        {
            Btransformed[edge] = log(b*d_in/(1-b*d_in));
            overall_count++;
        }

        // cout << b << " : " << Btransformed[edge] << " : " <<d_in<<endl;
    }
    cout<<"high count: "<<high_count<<endl;
    cout<<"overall_count: "<<overall_count<<endl;
    return Btransformed;
}


void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q,  unordered_map<int, double> &in_degrees) // or B?
{
    double b, d_in; 
    int v, f;
    pair <int, int> edge;
    for (auto &it : B)
    {

        b = it.second;
        edge = it.first;
        v = edge.second;
        d_in = in_degrees[v];
        f = node_to_feat[edge.second].size();
        if ((2-2*b*d_in)/(1-2*b*d_in) < 0)
        {
            Q[edge] = inf;
        }
        else
        {
            Q[edge] = (1/f) * log((2-2*b*d_in)/(1-2*b*d_in));
        }
        
    }
}

double calculate_MC_spread(DiGraph G, edge_prob P, unordered_set <int> S, int I)
{

    double overall_spread = 0.0;
    double tol = 0.00001; // from SIMPATH code

    for (size_t iter = 0; iter < I; iter++)
    {
        double spread = 0; // one simulation spread

        queue<int> T;
        map <int, NodeParams> Q;
        
        spread += S.size();
        

        for (unordered_set <int>::iterator it = S.begin(); it!=S.end(); it++) // iterate over the seed set
        {
            int uid = *it, v;
            // iterating through adjacent vertices
            out_edge_iter ei, e_end;
            for (boost::tie(ei, e_end) = out_edges(uid, G); ei!=e_end; ++ei) 
            {
                int v = target(*ei, G); // ID of the adj vertex?
                if (find(S.begin(), S.end(), v) == S.end())
                {
                    // Not in seed set
                    if (Q.find(v) == Q.end())
                    {
                        // Not been visited before
                        NodeParams np;
                        np.active = false;
                        pair <int, int> edge(source(*ei, G), target(*ei, G));
                        np.in_weight_sum = P[edge];
                        np.threshold = ((float)(rand() % 1001))/(float)1000; // from simpath

                        Q.insert(make_pair(v, np));
                        T.push(v); // to be processed as active or not                        
                    }
                    else
                    {
                        NodeParams& np = Q[v];
                        pair <int, int> edge(source(*ei, G), target(*ei, G));
                        np.in_weight_sum += P[edge];
                    }

                }
            }
        }
        // Time to process and continue
        while(! T.empty())
        {
            int u = T.front();
            
            // cout << "T.size " << T.size() << endl; // Debug
            NodeParams& np = Q.find(u)->second; // must have visited
            
            if (np.active == false && np.in_weight_sum >= np.threshold + tol) 
            {
                // active-able
                np.active = true;
                spread++;
                out_edge_iter ei, e_end;
                for (boost::tie(ei, e_end) = out_edges(u, G); ei!=e_end; ++ei) 
                {
                    // loop through neighbours of an active node
                    int v = target(*ei, G);
                    if (find(S.begin(), S.end(), v) == S.end()) continue; // IN seed set

                    if (Q.find(v) != Q.end()) // unseen node
                    {
                        NodeParams& np_v = Q[v]; // does this work?
                        np_v.threshold = ((float)(rand() % 1001))/(float)1000;
                        np_v.active = false;
                        pair <int, int> edge(source(*ei, G), target(*ei, G));
                        np_v.in_weight_sum = P[edge];
                        T.push(v);
                    }
                    else
                    {
                        NodeParams& np_v = Q[v];
                        if (np_v.active == false)
                        {
                            pair <int, int> edge(source(*ei, G), target(*ei, G));
                            np_v.in_weight_sum += P[edge];
                            T.push(v); // could be active now
                            if (np_v.in_weight_sum - 1 > tol) 
                            {
                                cout << "Something wrong, the inweight for a node is > 1. (w, inweight) = " << v << ", " << np_v.in_weight_sum - 1<< endl;
                            }
                        }
                    }
                }
            }

            T.pop(); // processed it now   

        }  // endwhile
        overall_spread += spread/I;
    }
    return overall_spread;     
}

pair<vector<int>, unordered_map<int, double> >  greedy(DiGraph G, edge_prob base, edge_prob Btransformed, edge_prob Q, unordered_set<int> S, unordered_map<int,
        vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> Phi, int K, int I, unordered_map <int, double> &in_degrees)
{
    vector <int> F;
    edge_prob P;
    unordered_map<int, bool> selected;
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    unordered_map<int, double> influence;
    vector <int> no_selected_features = {-1}; 
    P = init_probs(G, Btransformed, in_degrees);
    cout<<"Prob (0, 1): " << P[make_pair(0, 1)] << endl; // should be increasing
    cout<<"Prob (0, 2): " << P[make_pair(0, 2)] << endl; // should be increasing
    cout<<"Prob (0, 3): " << P[make_pair(0, 3)] << endl; // should be increasing
    // wrong. should be 1/Fv sigmoid of Bt, let's just say it's B.copy
    // however, does not limit the values to 1/fv

    while (F.size() < K) 
    {
        max_spread = -1;
        printf("it = %i; ", (int)F.size() + 1);
        fflush(stdout);
        for (auto &f: Phi) {
            // cout << f << " ";
            fflush(stdout);
            if (not selected[f]) {
                F.push_back(f);
                changed = increase_probabilities(G, Btransformed, Q, node_to_feat, F, feat_to_edges[f], P, in_degrees);
    // double calculate_MC_spread(DiGraph G, edge_prob P, vector <int> S, unordered_map <int, vector<int> > node_to_feat, int I)
                spread = calculate_MC_spread(G, P, S, I);
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
        printf("f = %i; spread = %.4f\n", max_feature, max_spread);
        increase_probabilities(G, Btransformed, Q, node_to_feat, F, feat_to_edges[max_feature], P, in_degrees);
        // monitor the value of one edge
        cout<<"Prob (0, 1): " << P[make_pair(0, 1)] << endl; // should be increasing
        cout<<"Prob (0, 2): " << P[make_pair(0, 2)] << endl; // should be increasing
        cout<<"Prob (0, 3): " << P[make_pair(0, 3)] << endl; // should be increasing
        influence[F.size()] = max_spread;
}
    return make_pair(F, influence);
}


double sigmoid(double x) // scalar. No need to vectorize?
{
    return (double) 1./(1.0 + exp(-x));
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

// copy-paste
void decrease_probabilities(edge_prob changed, edge_prob &P) {
    for (auto &item: changed) {
        pair<int, int> edge = item.first;
        double p = item.second;
        P[edge] = p;
    }
}

unordered_map<int, double> save_degrees(DiGraph G)
{
    unordered_map<int, double> in_degrees;
    vertex_iter vi, v_end;
    int in_d;
    // FILE* deg_file; - can load instead of recomputing
    // deg_file = fopen("toy_degrees.txt", "w"); 

    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi)
    {
        in_d = boost::in_degree(*vi, G);
        in_degrees.insert(make_pair(*vi, in_d));
        // fprintf(deg_file, "%d %d\n", *vi, in_d);
    }
    // fclose(deg_file);
    return in_degrees;
}


// calculate_MCPlus()
// calculate_simpath()
// calculate_rrset()