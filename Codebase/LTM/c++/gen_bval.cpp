#include "helpers.h"

edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B, unordered_map<int, double> &in_degrees);
void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q, unordered_map<int, double> &in_degrees); // or B?
double calculate_MC_spread(DiGraph G, edge_prob P, unordered_map <int, vector<int> > node_to_feat);
pair<vector<int>, unordered_map<int, double> >  greedy(DiGraph G, edge_prob base, edge_prob Btransformed, edge_prob Q, unordered_set<int> S, unordered_map<int, vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> Phi, int K, int I, unordered_map <int, double> &in_degrees);
edge_prob increase_probabilities(DiGraph G, edge_prob B, edge_prob Q, unordered_map<int, vector<int> > node_to_feat, vector<int> F, vector<pair<int, int> > E, edge_prob &P, unordered_map <int, double> &in_degrees);
void decrease_probabilities(edge_prob changed, edge_prob &P);
edge_prob init_probs(DiGraph, edge_prob ,edge_prob, unordered_map <int, vector<int> >);


int analyze_data(DiGraph G, edge_prob B)
{
    for (auto &it: B)
    {
         double b; int v;
    double d_in;
    pair <int, int> edge;
    for (auto &it : B)
    {
        b = it.second;
        edge = it.first;
        v = edge.second;
        d_in = in_degrees[v];
   
    }
    
}


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
    
    return 0;
}





unordered_map<int, double> save_degrees(DiGraph G)
{
    unordered_map<int, double> in_degrees;
    vertex_iter vi, v_end;
    int in_d;
    for (boost::tie(vi, v_end) = boost::vertices(G); vi != v_end; ++vi)
    {
        in_d = boost::in_degree(*vi, G);
        in_degrees.insert(make_pair(*vi, in_d));
    }
    return in_degrees;
}