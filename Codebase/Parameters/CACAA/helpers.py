import os
import networkx as nx
import numpy as np
import csv
import pickle

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
    vec = np.where(vec < theta, False, True)
    return vec

def thresh_one_user(vec, theta):
    
    vec = vec.strip().split(' ')
    vec = np.array(vec[1:], dtype=np.float)
    vec = np.where(vec < theta, False, True)
    return vec

def create_vecs(args):

    print("creating vecs for topics")

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

def check_sim(vec1, vec2, nbits=1):
    diff = np.count_nonzero(vec1!=vec2)

    # print("vector dif = ", diff)
    if diff < nbits:
        return True
    else:
        return False