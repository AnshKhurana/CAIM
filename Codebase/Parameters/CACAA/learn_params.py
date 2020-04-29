import csv
import numpy as np

from helpers import *

def get_aux_vk(args):

    print("learning aux parameters...")
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
    
    for log in logs:
      
        # list
        # u_id, topic_id, timestamp for v performing action a
        [v, a, t_v] = [int(x) for x in log]

        if (v, a) in current_table.keys():
            print("same post performed again", v, a)
            current_table[(v, a)] = t_v
        else:
             
            n[v] += 1
            # print("For node ", v)
            # print("Out nodes: ", end = " ")
            for w in g.successors(v):
                # print(w, end=" ")
                alpha_wa = get_alpha(user_features[w], topic_features[a])
                C2[(v, w)] += alpha_wa
                C4[(v, w)] += alpha_wa**2
            
            parents = []
            count = 0
            for u, act in current_table.keys():
                count += 1
                if g.has_edge(u, v) and check_sim(topic_features[act], topic_features[a], nbits=args.nbits) and (t_v > current_table[(u, act)]) and args.tau > (t_v - current_table[(u, act)]):
                    parents.append(u)

            d =  len(parents) 
            
            # print("Parents: ", end = " ")
            for u in parents:
                # print(u, end=" ")
                alpha_va = get_alpha(user_features[v], topic_features[a])
                C1[(u, v)] += alpha_va/d
                C3[(u, v)] += 1/d
            

            current_table[(v, a)] = t_v
            # print()
            # print("Size of current log: ", count) # progress 
            # print(alpha)
            # Update single element coeffs first

    print(len(C1.keys()))
    print(len(C2.keys()))
    print("aux parameters done")
    return g, n, C1, C2, C3, C4


def learn_params(args, g, n, C1, C2, C3, C4):


    # base_file = open(os.path.join(args.data_dir, args.exp, 'base_weights.txt'), 'w')
    # q_file = open(os.path.join(args.data_dir, args.exp, "marg_weights.txt"), "w")
    
    b = dict()
    q = dict()

    for edge in g.edges:
        u, v = edge # from u to v direction

        q1 = (C1[(u, v)] - 1/n[u]*C2[(u, v)]*C3[(u, v)]) / (C4[(u, v)] - 1/n[u]*(C2[(u, v)]**2))
        p = 1/n[u] * C3[(u, v)] - q1/n[u]*C2[(u, v)]

        q1 = min(1, max(0, q1))
        p = min(1, max(0, p))

        b[(u, v)] = p
        q[(u, v)] = q1
        # base_file.write("%d %d %f\n" % (u, v, p))
        # q_file.write("%d %d %f\n" % (u, v, q))

    # base_file.close()
    # q_file.close()

    print("Weights assigned to edges.")
        # formula for p, q and then write to text file
    return b, q 


def get_aux_cit(args):

    print("getting aux parameters...")

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

    published = dict()
    cited = dict()

    for log in logs:
        [u, v, c, p] = [int(x) for x in log]

        # if v == 1344:
        #     print(c)

        if u in published.keys():
            published[u].add(c)
        else:
            published[u] = set()
            published[u].add(c)
        
        if v in published.keys():
            published[v].add(p)
        else:
            published[v] = set()
            published[v].add(p)

        if v in cited.keys():
            cited[v].add(c)
        else:
            cited[v] = set()
            cited[v].add(c)
        
    for v in cited.keys():
        # people who have cited a paper
        # print("For node ", v)
        
        for msg in cited[v]:
            # print("who cited ", msg)
            # for each action
            for w in g.successors(v):
                alpha_wa = get_alpha(user_features[w], topic_features[msg])
                C2[(v, w)] += alpha_wa
                C4[(v, w)] += alpha_wa**2

            parents = []
            for u in g.predecessors(v):
                
                # There must be at least one parent for each node.
                # similar_cited = False
                similar_published = False

                # for paper in cited[u]:
                #     if check_sim(topic_features[paper], topic_features[msg], args.nbits):
                #         similar_cited = True
                #         break
                
                # if not similar_cited:
                # for paper in published[u]:
                #     if check_sim(topic_features[paper], topic_features[msg], args.nbits):
                #         similar_published = True
                #         break
            
                # if similar_cited or similar_published:
                #     parents.append(u)

                # NON similarity based
                if u in published.keys() and  msg in published[u]:
                    parents.append(u)

            d = len(parents)
            alpha_va = get_alpha(user_features[v], topic_features[msg])
            # print("Alpha va: ", alpha_va)
            # print("Parents: ", end = " ")
            for u in parents:
                # print(u, end=" ")
                C1[(u, v)] += alpha_va * 1/d
                C3[(u, v)] += 1/d
            # print()

    print(len(C1.keys()))
    print(len(C2.keys()))

    for i in range(num_users):
        n[i] = len(published[i])

    print("aux parameters learned.")
    return g, n, C1, C2, C3, C4




def get_roc():

    # read base weights
    pass


