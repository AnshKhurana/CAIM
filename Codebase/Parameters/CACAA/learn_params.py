import csv
import numpy as np

from helpers import *

def get_aux(args):

    logfile =  open(args.log_file, newline='')
    logs = csv.reader(logfile, delimiter=' ')
    num_users = get_num_users(args)
    g = read_graph(args.edge_list)  

    # store and read all features, for ease of alpha?
    topic_features = np.load(args.topic_features)
    user_features = np.load(args.user_features)

    # INIT
    # using row vectors
    n = np.zeros((num_users, 1))  # maps user ID to number of actions performed

    # storing coefficients for each edge, 
    # using dict because the data is sparse
    C1 = dict()
    C2 = dict()
    C3 = dict()
    C4 = dict()
    
    # init edge parameters
    for u, v in g.edges:
        C1[(u, v)] = 0
        C2[(u, v)] = 0
        C3[(u, v)] = 0
        C4[(u, v)] = 0

    # One large current_table
    # split logfile on per action basis to speed things up
    current_table = dict()
    # map user, action to time
    count = 1
    for log in logs:
        print("Log number: ", count) # progress 
        count+=1
        # list
        # u_id, topic_id, timestamp for v performing action a
        [v, a, t_v] = [int(x) for x in log]

        if (v, a) in current_table.keys():
            print("same post performed again", v, a)
            current_table[(v, a)] = t_v
        else:
             
            n[v] += 1
            print("For node ", v)
            # print("Out nodes: ", end = " ")
            for w in g.successors(v):
                # print(w, end=" ")
                alpha_wa = get_alpha(user_features[w], topic_features[a])
                C2[(v, w)] += alpha_wa
                C4[(v, w)] += alpha_wa**2
            
            parents = []
            for u, act in current_table.keys():
                if g.has_edge(u, v) and check_sim(topic_features[act], topic_features[a], nbits=50) and (t_v > current_table[u, act]) and args.tau > (t_v - current_table[u, act]):
                    parents.append(u)

            d =  len(parents) 
            
            print("Parents: ", end = " ")

            
            for u in parents:
                print(u, end=" ")
                alpha_va = get_alpha(user_features[v], topic_features[a])
                C1[(u, v)] += alpha_va/d
                C3[(u, v)] += 1/d
            

            current_table[(v, a)] = t_v
            print()
            # print(alpha)
            # Update single element coeffs first

    print(len(C1.keys()))
    print(len(C2.keys()))
    return g, n, C1, C2, C3, C4


def learn_params(args, g, n, C1, C2, C3, C4):


    base_file = open(os.path.join(args.data_dir, 'base_weights.txt'), 'w')
    q_file = open(os.path.join(args.data_dir, "marg_weights.txt"), "w")
    
    for edge in g.edges:
        u, v = edge # from u to v direction

        q = (C1[(u, v)] - 1/n[u]*C2[(u, v)]*C3[(u, v)]) / (C4[(u, v)] - 1/n[u]*(C2[(u, v)]**2))
        p = 1/n[u] * C3[(u, v)] - q/n[u]*C2[(u, v)]

        

        base_file.write("%d %d %f\n" % (u, v, p))
        q_file.write("%d %d %f\n" % (u, v, q))

        # formula for p, q and then write to text file


