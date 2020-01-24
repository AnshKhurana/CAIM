#include "helpers.h"
#define inf 100


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

pair<vector<int>, unordered_map<int, double> >  greedy_beam(DiGraph G, edge_prob base, edge_prob Btransformed, edge_prob Q, unordered_set<int> S, unordered_map<int,
        vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> Phi, int K, int I, unordered_map <int, double> &in_degrees, edge_prob &P)
{
    vector <int> F;
    unordered_map<int, bool> selected;
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    unordered_map<int, double> influence;
    int choice_index=0;
    vector <int> no_selected_features = {-1}; 
    P = init_probs(G, Btransformed, in_degrees);
    // wrong. should be 1/Fv sigmoid of Bt, let's just say it's B.copy
    // however, does not limit the values to 1/fv

    printf("it = 0; ");
    double first_spread = calculate_MC_spread(G, P, S, I);
    cout<<"spread = "<<first_spread<<endl;
    influence[0] = first_spread;
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
                    choice_index++;
                }
                decrease_probabilities(changed, P);
                F.pop_back();
            }
        }
        F.push_back(max_feature);
        selected[max_feature] = true;
        printf("f = %i; spread = %.4f\n", max_feature, max_spread);
        increase_probabilities(G, Btransformed, Q, node_to_feat, F, feat_to_edges[max_feature], P, in_degrees); // P by ref
        // monitor the value of one edge
        cout<<"Number of times the choice changed: "<<choice_index<<endl;
        choice_index = 0;
        influence[F.size()] = max_spread;
    }
    return make_pair(F, influence);
}





// directly from ltm.cpp

int main(int argc, char const *argv[])
{
    
    // Declarations
    unordered_map <int, vector<int> > node_to_feat;
    unordered_map<int, vector<pair<int, int> > > feat_to_edges;
    edge_prob B, Q, P; // P for by reference
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
    in_degrees = save_in_degrees(G);
    edge_prob Bt =  transform_probabilties(node_to_feat, B, in_degrees);
    
    if (q_file == "-")
    {
        gen_q(B, node_to_feat, Q, in_degrees); // or B?
        //  Use same as B, no constraints at all
        // Q.insert(B.begin(), B.end());
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


    // cout<<"Out degree of every seed: ";

    // for (auto &node: S) 
    // {
    //     cout << boost::out_degree(node, G) << " ";
    // }

    // cout << endl;

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
    FILE *results_file; // variables for files


    // Time to estimate the spread
    double init_mc_spread = calculate_MC_spread(G, P, S, I);
    cout<<"Init MC spread = "<<init_mc_spread<<endl;

    start = clock();

    // sets up result_feature_set
    if (strcmp(algo_name.c_str(), "simpath") == 0)
    {
        cout << "Starting algo: " <<"simpath"<< endl;
        boost::tie(result_feature_set, influence) = simpath(G, B, Bt, Q, S, node_to_feat, feat_to_edges, Phi, K, I,  1e-5, in_degrees, P);
    }
    else if (strcmp(algo_name.c_str(), "rr") == 0)
    {
        //...   Future extension
    }
    else if (strcmp(algo_name.c_str(), "simplus") == 0)
    {
        cout << "Starting algo: " <<"simplus"<< endl;
        boost::tie(result_feature_set, influence) = simpathPlus(G, B, Bt, Q, S, node_to_feat, feat_to_edges, Phi, K, I,  1e-3, in_degrees, P);
    }
    else
    {
        cout << "Starting algo: " <<"greedy"<< endl;
        boost::tie(result_feature_set, influence) = greedy(G, B, Bt, Q, S, node_to_feat, feat_to_edges, Phi, K, I, in_degrees, P);
    }
    finish = clock();
    double exec_time = (double) (finish - start)/CLOCKS_PER_SEC;
    cout << "Execution time = " << exec_time << " secs.\n";

    final_spread = calculate_MC_spread(G, P, S, I);
    cout<<"Final spread value: "<<final_spread<<endl;

    results_file = fopen(save_result.c_str(), "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
   
    // save parameters too! 
    fprintf(results_file, "Setup file: %s\n", setup_file.c_str());
    fprintf(results_file, "group_number K I\n");
    fprintf(results_file, "%d %d %d\n", group_number, K, I);
    fprintf(results_file, "algo_name: %s", algo_name.c_str());
    for (auto &node: S) 
    {
        fprintf(results_file, "%d ", node);
    }
    fprintf(results_file, "\n");

    fprintf(results_file, "Execution time  = %lf secs.\n", exec_time);
    
    // final_spread = calculate_MC_spread(result_feature_set); // ... for LTM 
    fprintf(results_file, "k spread\n");
    for (int num = 0; num <= K; ++num) {
      fprintf(results_file, "%d %f\n", num, influence[num]);
      cout << num << ": " << influence[num]  << " spread" << endl;
    }

    fprintf(results_file, "The final spread value (MC calculated) = %f\n", final_spread);
    
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
