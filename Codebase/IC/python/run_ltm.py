# author: anshkhurana
# date: 12 Dec 2019

from __future__ import division
from copy import copy
import networkx as nx
import pandas as pd
import os
import time
import heapq
import random
import math
from collections import Counter
from itertools import chain, combinations
import argparse

parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument("-v", "--verbosity", help="increase output verbosity", action="store_true")
parser.add_argument('--version', action='version', version='%(prog)s 1.1')
parser.add_argument('--dataset', default='vk', help="Dataset to be used for the experiment")
parser.add_argument('-K', '--K', default=51, type=int, help="Number of features to be selected")
parser.add_argument('-SG', '--SG', type=int, help="Manually specify seed group number")
parser.add_argument('-a', '--algo', default='eub', help="Algorithm for calculating spread")
parser.add_argument('-I', '--I', default=10000, type=int, help="Number of MC simulations")
parser.add_argument('-bw', '--bw', default=3, type=int, help="BEAM_WIDTH")
parser.add_argument('--theta', type=float, default=1./40, help="Explore Update theta")

# LTM - NO CHANGE
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

def add_graph_attributes(G, filename):
    """
    Add features as node attributes and construct feature -> edges
    :param G: networkx graph
    :param filename: (u f1 f2 f3 ...); u is mandatory in graph -- the node
    f1 f2 f3 ... - arbitrary number of features
    :return: Ef: dictionary f -> edges that it affects
    """
    Ef = dict() # feature -> edges
    Nf = dict() # node -> features
    with open(filename) as f:
        for line in f: # for each node, list of features it belongs to
            d = line.split()
            u = int(d[0])
            features = d[1:]
            for f in features:
                Ef.setdefault(f, []).extend(G.in_edges(u)) # add feature-dependent edges
            #G.node[u]['Fu'] = features
            G.nodes[u]['Fu'] = features
            Nf[u] = features
    print('Read graph attributes')
    return Ef, Nf

# Ef[f] list of edges that are affected by the feature 'f'.
# Nf[u] list of features to which 'u' belongs.


def read_probabilities(filename, sep=' '):
    """
    Creates a dataframe object indexed by endpoints of edge (u, v)
    :param filename: (u, v, p)
    :param sep: separator in the file
    :return:
    """
    df = pd.read_csv(filename, sep=sep, header = None)
    print('Read probabilities')
    return df.set_index([0, 1])
# Read the base probabilites / some constants for each edge
# pandas dataframe


def read_groups(filename):
    """
    Reads groups' memberships.
    :param filename: string each line is group and its members
    :return: dictionary group to members
    """
    groups = dict()
    with open(filename) as f:
        for line in f:
            d = line.split()
            members = map(int, d[1:])
            groups[d[0]] = members
    print('Read groups')
    return groups

# Read *com.txt file. Group is a dict indexed by com name with members as the nodes


def increase_probabilities(G, B, Q, F, E, P):
    """
    Increase probabilities of edges E depending on selected features F. Returns previous probabilities of changed edges.
    :param G: graph that has nodes attributes as features for nodes
    :param B: dataframe indexed by two endpoints of edge: base probabilities on edges
    :param Q: dataframe indexed by two endpoints of edge: product probabilities on edges
    :param F: selected set of features
    :param E: edges that require update
    :param K: number of required features
    :param P: final probabilities on edges (updated only Ef)
    :return:  previous probabilities of changed edges
    """
    raise "Not implemented, need to define for LTM"
    pass 


    
    # changed = dict() # changed edges and its previous probabilities
    # for e in E:
    #     changed[e] = float(P.loc[e]) # remember what edges changed
    #     hF = len(set(F).intersection(G.node[e[1]]['Fu']))/len(G.node[e[1]]['Fu']) # function h(F)
    #     q = float(Q.loc[e])
    #     b = float(B.loc[e])
    #     P.loc[e] = min(hF*q + b, 1) # final probabilities p = h(F)*q + b
    # return changed




def inc_prob_set(G, B, Q, F, Ef, P):
    """
    Set version of increase_probabilites
    """
    
    P = B.copy()
    E = []
    for f in F:
        E.extend(Ef[f])
    
    increase_probabilities(G, B, Q, F, E, P):
