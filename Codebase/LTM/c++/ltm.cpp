#include "helpers.h"
#define inf 100 
// for simgoid sgm(100) ~ 1
// borrow print and read functions from explore-update
// edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B)
edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B);
void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q); // or B?
double sigmoid (double);
double calculate_MC_spread(DiGraph G, edge_prob P, unordered_map <int, vector<int> > node_to_feat);

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
        cout << dataset_file << " " << b_file << " " << " " << q_file << " " <<
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
    edge_prob Bt =  transform_probabilties(node_to_feat, B);
    if (q_file == "-")
    {
        gen_q(B, node_to_feat, Q); // or B?
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
        //...
    }
    finish = clock();


    results_file = fopen(save_result.c_str(), "w"); // SPECIFY OUTPUT FILE FOR TIME AND INFLUENCE SPREAD
    double exec_time = (double) (finish - start)/CLOCKS_PER_SEC;
    fprintf(results_file, "Execution time  = %f\n secs.", exec_time);
    cout << "Execution time = " << exec_time << " secs.";
    
    // final_spread = calculate_MC_spread(result_feature_set); // ... for LTM 

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

double sigmoid(double x) // scalar. No need to vectorize?
{
    return 1/(1 + exp(-x));
}

edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B)
{
    edge_prob Btransformed;
    double b;
    pair <int, int> edge;
    for (auto &it : B)
    {
        b = it.second;
        edge = it.first;
        int f = node_to_feat[edge.second].size();
        if (1-b*f < 0)
        {
            Btransformed[edge] = inf;
        }
        else
        {
            Btransformed[edge] = log(b*f/(1-b*f));
        }

        // cout << b << " : " << Btransformed[edge] << " : " << f<<endl;
    }
    return Btransformed;
}


void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q) // or B?
{
    double b;
    pair <int, int> edge;
    for (auto &it : B)
    {

        b = it.second;
        edge = it.first;
        int f = node_to_feat[edge.second].size();
        if ((1-2*b*f)/(1-b*f) < 0)
        {
            Q[edge] = inf;
        }
        else
        {
            Q[edge] = log((1-2*b*f)/(1-b*f));
        }
        
    }
}

double calculate_MC_spread(DiGraph G, edge_prob P, vector <int> S, unordered_map <int, vector<int> > node_to_feat, int I)
{

    double overall_spread = 0.0;
    double tol = 0.00001; // from SIMPATH code

    for (size_t iter = 0; iter < I; iter++)
    {
        double spread = 0; // one simulation spread

        queue<int> T;
        map <int, NodeParams> Q;
        
        spread += S.size();
        

        for (size_t i = 0; i < S.size(); i++) // iterate over the seed set
        {
            int uid = S[i], v;
            // iterating through adjacent vertices
            edge_iter ei, e_end;
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
            
            cout << "T.size " << T.size() << endl; // Debug
            NodeParams& np = Q.find(u)->second; // must have visited
            
            if (np.active == false && np.in_weight_sum >= np.threshold + tol) 
            {
                // active-able
                np.active = true;
                spread++;
                edge_iter ei, e_end;
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

// calculate_MCPlus()
// calculate_simpath()
// calculate_rrset()