#include "helpers.h"
#define inf 100 
// for simgoid sgm(100) ~ 1
// borrow print and read functions from explore-update
// edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B)
edge_prob transform_probabilties (unordered_map<int, vector<int> > node_to_feat, edge_prob B, unordered_map<int, double> &in_degrees);
void gen_q(edge_prob B, unordered_map<int, vector<int> > node_to_feat, edge_prob &Q, unordered_map<int, double> &in_degrees); // or B?
double calculate_MC_spread(DiGraph G, edge_prob P, unordered_set <int> S, int I);
pair<vector<int>, unordered_map<int, double> >  greedy(DiGraph G, edge_prob base, edge_prob Q, unordered_set<int> S, unordered_map<int, vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> Phi, int K, int I, unordered_map <int, double> &in_degrees, edge_prob &P, int mode);
double simpath_spread(DiGraph, unordered_set <int>, edge_prob, double);
double simpath_spreadPhi(DiGraph, unordered_set <int>, edge_prob, double, unordered_map<int, vector <int> > node_to_feat,  set <int> &);
double forward_backward(DiGraph G, int v, edge_prob P, unordered_set <int> S,  double eta);
double forward_backward_wPhi(DiGraph G, int v, edge_prob P, unordered_set <int> S,  double eta, unordered_map <int, vector <int> > node_to_feat,  set <int> &phi);

pair<vector<int>, unordered_map<int, double> >  simpath(DiGraph G, edge_prob base, edge_prob Q, unordered_set<int> S,
  unordered_map<int, vector<int> > node_to_feat,
  unordered_map<int, vector<pair<int, int> > > feat_to_edges,
  vector<int> Phi, int K, int I, double eta, unordered_map <int, double> &in_degrees, edge_prob &P, int mode);

pair<vector<int>, unordered_map<int, double> >  simpathPlus(DiGraph G, edge_prob base, edge_prob Q, unordered_set<int> S,
  unordered_map<int, vector<int> > node_to_feat,
  unordered_map<int, vector<pair<int, int> > > feat_to_edges,
  vector<int> Phi, int K, int I, double eta, unordered_map <int, double> &in_degrees, edge_prob &P, int mode);



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
    cout<<"SEED SET SIZE: "<<S.size()<<endl;

    FILE *results_file; // variables for files

    FILE* eta_file;
    eta_file = fopen("simpath_eta", "a");
    
    // Time to estimate the spread
    // inputting the simple base prob
    P = init_probs_no_transform(G, B, in_degrees);
    double init_mc_spread = calculate_MC_spread(G, P, S, I);
    cout<<"Init MC spread = "<<init_mc_spread<<endl;

    start = clock();

    // sets up result_feature_set
    if (strcmp(algo_name.c_str(), "simpath") == 0)
    {
        cout << "Starting algo: " <<"simpath"<< endl;
        boost::tie(result_feature_set, influence) = simpath(G, B, Q, S, node_to_feat, feat_to_edges, Phi, K, I,  eta, in_degrees, P, mode);
    }
    else if (strcmp(algo_name.c_str(), "rr") == 0)
    {
        //...   Future extension
    }
    else if (strcmp(algo_name.c_str(), "simplus") == 0)
    {
        cout << "Starting algo: " <<"simplus"<< endl;
        boost::tie(result_feature_set, influence) = simpathPlus(G, B, Q, S, node_to_feat, feat_to_edges, Phi, K, I,  1e-3, in_degrees, P, mode);
    }
    else
    {
        cout << "Starting algo: " <<"greedy"<< endl;
        boost::tie(result_feature_set, influence) = greedy(G, B, Q, S, node_to_feat, feat_to_edges, Phi, K, I, in_degrees, P, mode);
    }
    finish = clock();
    double exec_time = (double) (finish - start)/CLOCKS_PER_SEC;
    cout << "Execution time = " << exec_time << " secs.\n";
    
    fprintf(eta_file, "eta: %lf\n", eta);
    fprintf(eta_file, "Execution time  = %lf secs.\n", exec_time);

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

pair<vector<int>, unordered_map<int, double> >  simpath(DiGraph G, edge_prob B,
    edge_prob Q, unordered_set<int> S,
  unordered_map<int, vector<int> > node_to_feat,
  unordered_map<int, vector<pair<int, int> > > feat_to_edges,
  vector<int> Phi, int K, int I, double eta, unordered_map <int, double> &in_degrees, edge_prob &P, int mode=0)

{

    cout<<"using eta: "<<eta<<endl;
    // plotting parameters
    clock_t start, finish;
    int report_interval = 2;

    FILE *time_file, *spread_file;

    if (mode != 0)
    {
        cout<<"Running to report results according to mode: "<<mode<<endl;

        if (mode==1)
        {
            spread_file = fopen("simpath_spread", "w");
        }
    }

    if (mode == 2)
    {
        start = clock();
        time_file = fopen("simpath_time", "w");
    }

    vector <int> F;
    unordered_map<int, bool> selected;
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    unordered_map<int, double> influence;
    vector <int> no_selected_features = {-1}; 
    
    P = init_probs_no_transform(G, B, in_degrees);
    cout<<"no seg fault\n";
    int choice_index = 0;
    
    // wrong. should be 1/Fv sigmoid of Bt, let's just say it's B.copy
    // however, does not limit the values to 1/fv

    printf("it = 0; ");
    double first_spread = simpath_spread(G, S, P, eta);
    // cout<<"first done\n";
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
                changed = increase_probabilities_no_transform(G, B, Q, node_to_feat, F, feat_to_edges[f], P, in_degrees);
                spread = simpath_spread(G, S, P, eta);
                
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
        // cout<<mode<<endl;
        if (mode == 2)
        {
            finish = clock();
            double exec_time = (double) (finish - start)/CLOCKS_PER_SEC;
            cout<<"F.size() "<<F.size()<<"; Time elapsed: "<<exec_time<<endl;
            fprintf(time_file, "F.size() %lu; Time elapsed: %0.4f\n", F.size(), exec_time);
        }
        printf("f = %i; spread = %.4f\n", max_feature, max_spread);
        increase_probabilities_no_transform(G, B, Q, node_to_feat, F, feat_to_edges[max_feature], P, in_degrees);

        if (mode == 1)
        {
            // calc mc spread after the permanent increase. Slows down the algorithm
            if (F.size()%report_interval == 0)
            {
                double mc_spread = calculate_MC_spread(G, P, S, I);   
                printf("|F| = %lu; mc_spread = %.4f\n", F.size(), mc_spread);
                fprintf(spread_file, "|F| = %lu; mc_spread = %.4f\n", F.size(), mc_spread);
            }
        }

        // monitor the value of one edge
        cout<<"Number of times the choice changed: "<<choice_index<<endl;
        choice_index = 0;
        influence[F.size()] = max_spread;
    }
    return make_pair(F, influence);

}


pair<vector<int>, unordered_map<int, double> >  simpathPlus(DiGraph G, edge_prob base, edge_prob Q, unordered_set<int> S,
  unordered_map<int, vector<int> > node_to_feat,
  unordered_map<int, vector<pair<int, int> > > feat_to_edges,
  vector<int> Phi, int K, int I, double eta, unordered_map <int, double> &in_degrees, edge_prob &P, int mode=0)
{

    // to remove later
    edge_prob Btransformed;

    vector <int> F;
    unordered_map<int, bool> selected;
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    unordered_map<int, double> influence;
    vector <int> no_selected_features = {-1}; 
    P = init_probs_no_transform(G, Btransformed, in_degrees);
    // cout<<"Prob (0, 1): " << P[make_pair(0, 1)] << endl; // should be increasing
    // cout<<"Prob (0, 2): " << P[make_pair(0, 2)] << endl; // should be increasing
    // cout<<"Prob (0, 3): " << P[make_pair(0, 3)] << endl; // should be increasing
    // wrong. should be 1/Fv sigmoid of Bt, let's just say it's B.copy
    // however, does not limit the values to 1/fv

    printf("it = 0; ");
    vector <int> curPhi;
    cout<<"Complete feature set size: "<<Phi.size()<<endl;
    set <int> nextPhi;
    double first_spread = simpath_spreadPhi(G, S, P, eta, node_to_feat, nextPhi);
    cout<<"first done\n";
    cout<<"spread = "<<first_spread<<endl;
    influence[0] = first_spread;

    curPhi.assign(nextPhi.begin(), nextPhi.end());
    nextPhi.clear();
    cout<<"feature set size for 1st iteration: "<<curPhi.size()<<endl;
    while (F.size() < K) 
    {
        max_spread = -1;
        printf("it = %i; ", (int)F.size() + 1);
        fflush(stdout);
        for (auto &f: curPhi) {
            // cout << f << " ";
            fflush(stdout);
            if (not selected[f]) {
                F.push_back(f);
                changed = increase_probabilities_no_transform(G, Btransformed, Q, node_to_feat, F, feat_to_edges[f], P, in_degrees);
                spread = simpath_spreadPhi(G, S, P, eta, node_to_feat, nextPhi);
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
        increase_probabilities_no_transform(G, Btransformed, Q, node_to_feat, F, feat_to_edges[max_feature], P, in_degrees);
        
        // Time to update set Phi for next iteration
        curPhi.clear(); // unnecessary? remove to speed-up
        curPhi.assign(nextPhi.begin(), nextPhi.end());
        nextPhi.clear();
        cout<<"Size of feature set for the next iteration: "<<curPhi.size()<<endl;
        
        // monitor the value of one edge
        cout<<"Prob (0, 1): " << P[make_pair(0, 1)] << endl; // should be increasing
        cout<<"Prob (0, 2): " << P[make_pair(0, 2)] << endl; // should be increasing
        cout<<"Prob (0, 3): " << P[make_pair(0, 3)] << endl; // should be increasing
        influence[F.size()] = max_spread;
    }
    return make_pair(F, influence);

}


double simpath_spreadPhi(DiGraph G, unordered_set <int> S, edge_prob P, double eta, unordered_map<int, vector <int> > node_to_feat,  set <int> &nextPhi)
{
    double spread = 0.0;
    // unordered_set <int> ignored_vertices(S);

    for (auto &s: S)
    {
        // ignored_vertices.erase(s);
        // cout<<"before fb\n";
        spread += forward_backward_wPhi(G, s, P, S, eta, node_to_feat, nextPhi); 
        // cout<<"iterating\n";
        // no need to create an induced subgraph since incoming edges for the
        // current seed elements have already been removed
        // ignored_vertices.insert(s);
    }
    
    return spread;
    
}

double simpath_spread(DiGraph G, unordered_set <int> S, edge_prob P, double eta=1e-6)
{

    double spread = 0.0;
    // unordered_set <int> ignored_vertices(S);

    for (auto &s: S)
    {

        if (boost::out_degree(s, G) < 1)
        {
            spread += 1.0;
            continue;
        }
        // ignored_vertices.erase(s);
        // cout<<"before fb\n";
        spread += forward_backward(G, s, P, S, eta); 
        // cout<<"iterating\n";
        // no need to create an induced subgraph since incoming edges for the
        // current seed elements have already been removed
        // ignored_vertices.insert(s);
    }
    
    return spread;
    
}



double forward_backward_wPhi(DiGraph G, int v, edge_prob P, unordered_set <int> S,  double eta, unordered_map <int, vector <int> > node_to_feat,  set <int> &nextPhi)
{
    double spd = 1.0, pp = 1.0;
    // need xNode DS for the node?
    // get xNode DS for s
    // whatever

    vector <int> Q; // Q using vector - in place for xNode queue
    set <int> Q1; //why not unordered?
    map <int, set <int> > D;
    
    Q.push_back(v); 
    Q1.insert(v);

    unordered_map <int, double> PrPtill;

    PrPtill.insert(make_pair(v, pp));

    while (! Q.empty()) // main_loop
    {
        int last_v = -1;

        // FORWARD pass
        while(true)
        {
            int x = Q.back(); // ID of curr Node
            if (x == last_v)
            {
                cout<<"Notice when this happens\n"; //only 1 iteration? what depth first?
                break;

            }

            last_v = x;

            pp =  PrPtill[x];

            // loop through out-neighbours now
            out_edge_iter ei, e_end;
            for (boost::tie(ei, e_end) = out_edges(x, G); ei!=e_end; ++ei) 
            {
                int y = target(*ei, G);
                if (y == v) // y is just v, dataset might have self loops?
                {
                    D[x].insert(y);
                    cout<<"should never happen, no in_edges for v\n"; 
                    continue; // will never happen tho, no in_edges for v but don't know if boost updated
                }
                if (Q1.find(y) != Q1.end()) // no cycles
                {
                    continue;
                }
                if (D[x].find(y) != D[x].end()) // y is an already-explored neighbor of x
                {
                    continue;
                }
                if (S.find(y) != S.end()) 
                { // y is a seed, induced subgraph check
                    D[x].insert(y);
                    continue;
                }
                // cout<<"Ok, not ignored\n";
                double ppnext = pp * P[make_pair(x, y)];
                
                if (ppnext < eta) // time to prune
                {
                    // cout<<"Pruned\n";
                    spd += ppnext;
                    // PrPtill[y] = ppnext; - no need to store since this will not be used in the future
                    D[x].insert(y);
                    nextPhi.insert(node_to_feat[y].begin(), node_to_feat[y].end());
                    // nextPhi.append(node_to_feat[y])
                    continue;
                }

                spd+= ppnext;
                PrPtill[y] = ppnext;

                Q.push_back(y);
                Q1.insert(y);
                D[x].insert(y); //explored
                break;
            } //endfor
        } //endForward
    
        // BACKWARD pass

        int pop_id = Q.back(); // ID of xNode u

        if (Q.size() == 1) // copy-paste
        {
            if (pop_id != v) 
            { // in the end one should get back to current s
                cout << "The only remaining node in Q is: " << pop_id << ", but not: " << v << endl;
                exit(1);
            } //endif

            // if (D[pop_id].size() < out_degree(v, G)) { // I have degrees :)
            //     // cout<<"have not explored all out neighbors of the start node, continue!!\n";
            //     continue;
            // } //endif
        }


        Q1.erase(pop_id);
        D.erase(pop_id);
        Q.pop_back();
    }
    return spd;
}



double forward_backward(DiGraph G, int v, edge_prob P, unordered_set <int> S,  double eta)
{
    double spd = 1.0, pp = 1.0;
    // need xNode DS for the node?
    // get xNode DS for s
    // whatever

    vector <int> Q; // Q using vector - in place for xNode queue
    set <int> Q1; //why not unordered?
    map <int, set <int> > D;
    
    Q.push_back(v); 
    Q1.insert(v);

    int mysterious_case = 0;

    unordered_map <int, double> PrPtill;

    PrPtill.insert(make_pair(v, pp));

    while (! Q.empty()) // main_loop
    {
        int last_v = -1;

        // FORWARD pass
        while(true)
        {
            int x = Q.back(); // ID of curr Node
            if (x == last_v)
            {
                // cout<<"Notice when this happens\n"; only 1 iteration? what depth first?
                break;
            }

            last_v = x;

            pp =  PrPtill[x];

            // loop through out-neighbours now
            out_edge_iter ei, e_end;
            for (boost::tie(ei, e_end) = out_edges(x, G); ei!=e_end; ++ei) 
            {
                int y = target(*ei, G);
                // if (x == v)
                // {
                //     cout<<"visiting neigh of seed ("<<x<<", "<<y<<")"<<"\n";
                // }
                
                if (y == v) // y is just v, dataset might have self loops?
                {
                    D[x].insert(y);
                    cout<<"should never happen, no in_edges for v\n"; 
                    continue; // will never happen tho, no in_edges for v but don't know if boost updated
                }
                if (Q1.find(y) != Q1.end()) // no cycles
                {

                    continue;
                }
                if (D[x].find(y) != D[x].end()) // y is an already-explored neighbor of x
                {
                    continue;
                }
                if (S.find(y) != S.end()) 
                { // y is a seed, induced subgraph check
                    cout<<"removing clear in-edges\n";
                    D[x].insert(y);
                    continue;
                }
                // cout<<"Ok, not ignored\n";
                double ppnext = pp * P[make_pair(x, y)];
                
                if (ppnext < eta) // time to prune
                {
                    // cout<<"Pruned\n";
                    spd += ppnext;
                    // PrPtill[y] = ppnext; - no need to store since this will not be used in the future
                    
                    D[x].insert(y);
                    // nextPhi.append(node_to_feat[y])
                    continue;
                }
                
                spd+= ppnext;
                PrPtill[y] = ppnext;

                Q.push_back(y);
                Q1.insert(y);
                
                D[x].insert(y); //explored
                break;
            } //endfor
        } //endForward
    
        // cout<<"Entering backward\n";
        // BACKWARD pass

        int pop_id = Q.back(); // ID of xNode u

        // if (pop_id == v)
        // {
        //     cout<<"how did this happen?\n";
        //     /* code */
        // }

        if (Q.size() == 1) // copy-paste
        {
            if (pop_id != v) 
            { // in the end one should get back to current s
                cout << "The only remaining node in Q is: " << pop_id << ", but not: " << v << endl;
                exit(1);
            } //endif

            if (D[pop_id].size() < boost::out_degree(pop_id, G)) { // I have degrees :)
                mysterious_case++;
                
                cout << "Seed being oberved: " << pop_id << " compare D[seed].size - "<<D[pop_id].size()<< " with "  <<boost::out_degree(pop_id, G)<<" have not explored all out neighbors of the start node, continue!!\n";
                // exit(1);
                // usleep(2000ß00);
                continue;
            } 
            // else
            // {

            //     cout<<"Not going into condition\n";
            //     cout<<"Values: size - "<<D[pop_id].size() << " " <<D[v].size() <<" out_degree - "<< boost::out_degree(pop_id, G) << "  " << boost::out_degree(v, G)<<endl;
            // }
        }


        Q1.erase(pop_id);
        D.erase(pop_id);
        Q.pop_back();
    }
    // cout<<"Encounted mystery: "<<mysterious_case<<" times.\n";
    return spd;
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

pair<vector<int>, unordered_map<int, double> >  greedy(DiGraph G, edge_prob B, edge_prob Q, unordered_set<int> S, unordered_map<int,
        vector<int> > node_to_feat, unordered_map<int, vector<pair<int, int> > > feat_to_edges, vector<int> Phi, int K, int I, unordered_map <int, double> &in_degrees, edge_prob &P, int mode=0)
{

    // plotting parameters

    clock_t start, finish;
    int report_interval = 2;

    FILE *time_file, *spread_file;

    if (mode != 0)
    {
        cout<<"Running to report results according to mode: "<<mode<<endl;
        if (mode == 1)
        {
            spread_file = fopen("greedy_spread", "w");
        }
    }

    if (mode == 2)
    {
        start = clock();
        time_file = fopen("greedy_time", "w");
    }


    vector <int> F;
    unordered_map<int, bool> selected;
    edge_prob changed;
    double spread, max_spread;
    int max_feature;
    unordered_map<int, double> influence;
    int choice_index=0;
    vector <int> no_selected_features = {-1}; 
    P = init_probs_no_transform(G, B, in_degrees);
    // cout<<"Prob (0, 1): " << P[make_pair(0, 1)] << endl; // should be increasing
    // cout<<"Prob (0, 2): " << P[make_pair(0, 2)] << endl; // should be increasing
    // cout<<"Prob (0, 3): " << P[make_pair(0, 3)] << endl; // should be increasing
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
                changed = increase_probabilities_no_transform(G, B, Q, node_to_feat, F, feat_to_edges[f], P, in_degrees);
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
        // cout<<mode<<endl;
        if (mode == 2)
        {
            finish = clock();
            double exec_time = (double) (finish - start)/CLOCKS_PER_SEC;
            cout<<"F.size() "<<F.size()<<"; Time elapsed: "<<exec_time<<endl;
            fprintf(time_file, "F.size() %lu; Time elapsed: %0.4f", F.size(), exec_time);
        }
        printf("f = %i; spread = %.4f\n", max_feature, max_spread);
        increase_probabilities_no_transform(G, B, Q, node_to_feat, F, feat_to_edges[max_feature], P, in_degrees);

        if (mode == 1)
        {
            fprintf(spread_file, "|F| = %lu; mc_spread = %.4f\n", F.size(), max_spread);    
        }
        
        // if (mode == 1)
        // {
        //     // calc mc spread after the permanent increase. Slows down the algorithm
        //     if (F.size()%report_interval == 0)
        //     {
        //         double mc_spread = calculate_MC_spread(G, P, S, I);   
        //         printf("|F| = %lu; mc_spread = %.4f\n", F.size(), mc_spread);
                
        //     }
        // }
        cout<<"Number of times the choice changed: "<<choice_index<<endl;
        choice_index = 0;
        influence[F.size()] = max_spread;
    }
    
    return make_pair(F, influence);
}




// calculate_MCPlus() - x
// calculate_rrset() - future workå