import csv
import numpy as np

from helpers import *

def get_aux(args):

    logfile =  open(args.log_file, newline='')
    logs = csv.reader(logfile, delimiter=' ')

    
    num_users = get_num_users(args)

    
    # using row vectors
    n = np.zeros((num_users, 1))  # maps user ID to number of actions performed
    
    # coeffs, per node
    C1v = np.zeros((num_users, 1))
    C2v = np.zeros((num_users, 1))


    g = read_graph(args.edge_list)  

    # store and read all features, for ease of alpha?
    topic_features = np.load(args.topic_features)
    user_features = np.load(args.user_features)


    # storing coefficients for each edge, 
    # using dict because the data is sparse
    C1vu = dict()
    C2vu = dict()
    
    # One large current_table
    # split logfile on per action basis to speed things up
    current_table = dict()
    # map user, action to time

    for log in logs:
    
        # list
        # u_id, topic_id, timestamp
        [u, a, t_u] = [int(x) for x in log]
        

        current_table[(u, a)] = t_u
        alpha = get_alpha(user_features[u], topic_features[a]) 
        # print(alpha)
        # Update single element coeffs first


        C1v[u] += alpha
        n[u] += 1
        C2v[u] += alpha**2

        parents = []
        for v, act in current_table.keys():
            if act != a:
                continue
            else:
                if g.has_edge(v, u):
                    parents.append(v)
                    # same action and outgoing edge exists

        d = len(parents) # need not store, useless qty
        # ASSUMPTION: no action is performed twice,
        # don't blame for bugs

        for v in parents:    
            if t_u > current_table[(v, a)]:
                # update params Cvu1, Cvu2
                
                C1vu[(v, u)] += get_alpha(args, a)/d
                C2vu[(v, u)] += 1/d
            else:
                # since in chrono order
                print("error")
                exit(1)
        
        

    # Once all aux coeffs are done, complete sanity check

    # code for getting params 
    print(len(C1vu.keys()))
    print(len(C2vu.keys()))
    return g, n, C1v, C2v, C1vu, C2vu


def learn_params(g, n, C1v, C2v, C1vu, C2vu):

    for edge in g.edges:
        u, v = edge # from u to v direction

        # formula for p, q and then write to text file

    
    