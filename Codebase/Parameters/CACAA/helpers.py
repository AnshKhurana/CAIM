import os
import networkx as nx
import numpy as np
import csv
import pickle


def get_actions_from_logs(logs, byuser, tbefore, sim_bits):

    """
    Check actions that are performed by u, such that it can influence v
    """


    actionset = []

    for u, au, tu in logs:

        if byuser != u or  tu > tbefore:
            continue            
        # some post by u, check if au is suitable for addition
        
        for a in actionset:
            if check_sim(a, au, nbits=sim_bits):
                continue
            else:
                actionset.append(au)

    # print("Found action from user")
    return actionset


def checklog(logs, u, a, t_v, sim_bits):
    
    for log in logs:
        if log[0] == u and check_sim(log[1], a, nbits=sim_bits) and log[2] < t_v:
            return True

    return False


def read_graph(filename, directed=True):
    """
    Create networkx graph reading file.
    :param filename: every line (u, v)
    :param directed: boolean
    :return: G
    """
    if not directed:
        G = nx.Graph()
    else:
        G = nx.DiGraph()
    with open(filename) as f:
        for line in f:
            d = line.split()
            G.add_edge(int(d[0]), int(d[1]))
    print('Read Graph')
    return G

def thresh_one(vec, theta):
    
    vec = vec.strip().split(' ')
    vec = np.array(vec, dtype=np.float)
    vec = np.where(vec > theta, True, False)
    return vec

def thresh_one_user(vec, theta):
    
    vec = vec.strip().split(' ')
    vec = np.array(vec[1:], dtype=np.float)
    vec = np.where(vec < theta, False, True)
    return vec

def create_vecs(args):

    print("creating vecs for topics")
    print("Topic threshold for creating vecs", args.topic_thr)
    with open(args.topic_features_input, 'r') as f:
        topic_list = f.readlines()

    print("Total number of actions", len(topic_list))    
    print(type(topic_list), type(topic_list[0]))
    
    topic_list = [thresh_one(x, args.topic_thr) for x in topic_list]
    
    topic_list = np.array(topic_list)

    print("Shape of topic np", topic_list.shape, topic_list.dtype)

    with open(os.path.join(args.data_dir, args.exp, 'topic_features.npy'), 'wb') as f:
        np.save(f, topic_list)
    
    print("creating vecs for user features")

    with open(args.user_features_input, 'r') as f:
        user_list = f.readlines()

    print("Total number of users", len(user_list))    
    # should match with the number of users

    print(type(user_list), type(user_list[0]))
    
    user_list = [thresh_one(x, args.topic_thr) for x in user_list]
    
    user_list = np.array(user_list)

    print("Shape of user topic np", user_list.shape, user_list.dtype)
    

    with open(os.path.join(args.data_dir, args.exp, 'user_features.npy'), 'wb') as f:
        np.save(f, user_list)    


def get_alpha(u_vec, t_vec): 
    # print("user vec", u_vec.shape)
    # print("topic vec", t_vec.shape)
    return np.sum(np.logical_and(u_vec, t_vec))


def get_num_users(args):

    user_set = set()
    edge_list = open(args.edge_list, newline='')
    edges = csv.reader(edge_list, delimiter=' ')

    for edge in edges:
        # print(edge)
        [u, v] = edge
        user_set.add(u)
        user_set.add(v)
    
    return len(user_set)

def check_sim(vec1, vec2, nbits):
    
    diff = np.count_nonzero(vec1!=vec2)

    # print("vector dif = ", diff)
    if diff < nbits:
        return True
    else:
        return False


def print_all(C1, C2, C3, C4):

    print("--------------------------------------")
    print()
    for u, v in C1.keys():
        print(u, v)
        print(C1[(u, v)], C2[(u, v)], C3[(u, v)], C4[(u, v)])
        print()

def transform_set(args):

    user_features = np.load(args.user_features)

    with open(os.path.join(args.data_dir, args.exp, 'mem.txt'), 'w') as f:

        for i in range(user_features.shape[0]):
            f.write(str(i) + " ")
            for j in range(args.num_topics):

                if user_features[i][j] == 1:
                    f.write(str(j) + " ")
            f.write("\n")

    with open(os.path.join(args.data_dir, args.exp, 'com.txt'), 'w') as f:

        for j in range(args.num_topics):
            f.write(str(j) + " ")

            for i in range(user_features.shape[0]):
                if user_features[i][j] == 1:
                    f.write(str(i) + " ")
            f.write("\n")
        
    

def save_weights(args, b, q):
 
    base_file = open(os.path.join(args.data_dir, args.exp, 'base_weights.txt'), 'w')
    q_file = open(os.path.join(args.data_dir, args.exp, "marg_weights.txt"), "w")
   
    for u, v in b.keys():
        p = b[(u, v)]
        q1 = q[(u, v)]

        base_file.write("%d %d %f\n" % (u, v, p))
        q_file.write("%d %d %f\n" % (u, v, q1))

    base_file.close()
    q_file.close()

    print("Weights saved in files.")