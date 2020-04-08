import os, csv
import numpy as np

def check_same():
    with open('../data/topic_features.npy', 'rb') as f:
        tf = np.load(f)

    print("all", tf.shape)

    print("unique rows:", np.unique(tf, axis=0).shape)


def check_num_users():

    print("From graph")
    user_set = set()
    edge_list = open('../data/vk_graph_lt.csv', newline='')
    edges = csv.reader(edge_list, delimiter=' ')

    for edge in edges:
        [u, v] = edge
        user_set.add(u)
        user_set.add(v)
    
    print(len(user_set))

    print("From user features ")

    with open('../data/user_preferences.csv', 'r') as f:
        user_list = f.readlines()

    print("Total number of users", len(user_list))    
    



if __name__ == "__main__":
    # check_same()
    check_num_users()