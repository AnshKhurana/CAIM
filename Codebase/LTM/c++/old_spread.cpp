#include "helpers.h"
#define inf 100 
// program to return MC spreads for runs
void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q,  unordered_map<int, double> &in_degrees) // or B?
{
    double b, d_in; 
    int v, f;
    pair <int, int> edge;
    double c = 5;
    int inf_count = 0, overall_count = 0; 
    for (auto &it : B)
    {

        b = it.second;
        edge = it.first;
        v = edge.second;
        d_in = in_degrees[v];
        f = node_to_feat[edge.second].size();
        if ((c-c*b*d_in)/(1-c*b*d_in) < 0)
        {
            Q[edge] = inf;
            inf_count++;
            overall_count++;
        }
        else
        {
            Q[edge] = (1.0/f) * log((c-c*b*d_in)/(1-c*b*d_in));
            overall_count++;
        }
        
    }
    cout<<"gen Q inf_count/overall_count :"<<inf_count<<"/"<<overall_count<<endl;
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

void  get_spread(DiGraph G, edge_prob B, edge_prob Q, unordered_set<int> S, unordered_map<int,
        vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> F, int K, int I, unordered_map <int, double> &in_degrees, edge_prob &P, int mode=0)
{
    
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    vector<int> cf;
    unordered_map<int, double> influence;
    int choice_index=0;
    vector <int> no_selected_features = {-1}; 
    P = init_probs(G, B, in_degrees);
    printf("it = 0; ");
    double first_spread = calculate_MC_spread(G, P, S, I);
    cout<<"spread = "<<first_spread<<endl;
    influence[0] = first_spread;
    for (auto &f: F) 
    {
        cf.push_back(f);
        changed = increase_probabilities(G, B, Q, node_to_feat, cf, feat_to_edges[f], P, in_degrees);
        spread = calculate_MC_spread(G, P, S, I);
        cout<<"f = "<<f<<" "<<"MC spread: "<<spread<<endl;
        decrease_probabilities(changed, P);
        increase_probabilities(G, B, Q, node_to_feat, cf, feat_to_edges[max_feature], P, in_degrees);
    }
}


int main(int argc, char const *argv[])
{
    
    // Declarations
    unordered_map <int, vector<int> > node_to_feat;
    unordered_map<int, vector<pair<int, int> > > feat_to_edges;
    edge_prob B, Q, P; // P for by reference
    unordered_map<int, unordered_set<int> > groups; // feat to nodes
    unordered_set<int> S; // Seed set
    double eta;
    int mode;
    int I, K, group_number; 
    clock_t start, finish;
    double final_spread;
    vector <int> result_feature_set{111, 109, 16, 19, 148, 144, 99, 61, 140, 20, 143, 102, 121, 104, 112, 45, 77, 75, 150, 0, 68, 60, 49, 134, 18, 34, 129, 72, 85, 36, 125, 29, 74, 107, 43, 4, 27, 106, 90, 66, 54, 69, 126, 39, 6, 1, 98, 83, 17, 131};
    unordered_map<int, double> influence; // for greedy algo
    unordered_map <int, double> in_degrees; // double to prevent divsion type casts
    string dataset_file, b_file, q_file, mem_file, algo_name, groups_file,
     seeds_file, save_dir, save_result, save_features, out_features_file, out_results_file;
    string setup_file = argv[1];
    if (argc > 1)
    {
        cout<<"Reading parameters...\n";
        
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
        getline(infile, line);
        eta = stof(line);
        getline(infile, line);
        mode = stoi(line);

        printf("eta: %lf\n", eta);
        
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

    DiGraph G = simple_read_graph(dataset_file);
    read_features(mem_file, G, node_to_feat, feat_to_edges);
    read_probabilities(b_file, B);
    in_degrees = save_in_degrees(G);
    
    
    if (q_file == "-")
    {
        gen_q(B, node_to_feat, Q, in_degrees); // or B?
        //  Use same as B, no constraints at all
        // Q.insert(B.begin(), B.end());
    }
    else
    {
        cout<<"Q file specified\n";
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
        read_seeds(seeds_file, S, 40); // only for VK network
    }
    // One cannot activate a seed node, remove incoming edges
    for (auto &node: S)
    {
        boost::clear_in_edges(node, G); // # smart move
    }


    cout << "S: ";

    for (auto &node: S) 
    {
        cout << node <<"w"<<boost::out_degree(node, G)<<" ";
    }
    cout << endl;

    cout << "I: " << I << endl;
    cout << "K: " << K << endl;
    cout<<"SEED SET SIZE: "<<S.size()<<endl;

    
    P = init_probs(G, B, in_degrees);
    double init_mc_spread = calculate_MC_spread(G, P, S, I);
    cout<<"Init MC spread = "<<init_mc_spread<<endl;

    get_spread(G, B, Q, S, node_to_feat, feat_to_edges, result_feature_set, K, I, in_degrees, P, mode);
     
    return 0;
}
