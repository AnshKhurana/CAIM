# author: sivanov
# date: 27 Oct 2015
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


def read_graph_OLD(filename, directed=True, sep=' ', header = None):
    """
    Create networkx graph using pandas.
    :param filename: every line (u, v)
    :param directed: boolean
    :param sep: separator in file
    :return
    """
    df = pd.read_csv(filename, sep=sep, header = header)
    if directed:
        G = nx.from_pandas_dataframe(df, 0, 1, create_using=nx.DiGraph())
    else:
        G = nx.from_pandas_dataframe(df, 0, 1)
    print('Read graph')
    return G

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
        for line in f:
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

def inc_prob_set(G, B, Q, F, Ef, P):
    """
    Set version of the increase probabilites function
    """
    cur = []
    for f in F:
        increase_probabilities(G, B, Q, cur + [f], Ef[f], P)
        cur = cur + [f]


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
    changed = dict() # changed edges and its previous probabilities
    for e in E:
        changed[e] = float(P.loc[e]) # remember what edges changed
        hF = len(set(F).intersection(G.node[e[1]]['Fu']))/len(G.node[e[1]]['Fu']) # function h(F)
        q = float(Q.loc[e])
        b = float(B.loc[e])
        P.loc[e] = min(hF*q + b, 1) # final probabilities p = h(F)*q + b
    return changed

def decrease_probabilities(changed, P):
    """
    Decrease probabilities of changed edges.
    :param changed: edge (u,v) -> probability
    :param P: dataframe with probabilities on edges
    :return:
    """
    for e in changed:
        P.loc[e] = changed[e]

def opt_calculate_MC_spread(G, S, P, I, thresh):
    """
    Returns influence spread in IC model from S with features F using I Monte-Carlo simulations.
    :param G: networkx graph
    :param S: list of seed set
    :param P: dataframe of probabilities
    :param I: integer of number of MC iterations
    :return: influence spread
    """
    spread = 0.
    runs = 0
    for _ in range(I):
        activated = dict(zip(G.nodes(), [False]*len(G)))
        for node in S:
            activated[node] = True
        T = [node for node in S]
        i = 0
        while i < len(T):
            v = T[i]
            for neighbor in G[v]:
                if not activated[neighbor]:
                    prob = float(P.loc[v, neighbor])
                    if random.random() < prob:
                        activated[neighbor] = True
                        T.append(neighbor)
            i += 1
        spread += len(T)
        runs +=1
    return spread/runs

def calculate_MC_spread(G, S, P, I):
    """
    Returns influence spread in IC model from S with features F using I Monte-Carlo simulations.
    :param G: networkx graph
    :param S: list of seed set
    :param P: dataframe of probabilities
    :param I: integer of number of MC iterations
    :return: influence spread
    """
    spread = 0.
    for _ in range(I):
        activated = dict(zip(G.nodes(), [False]*len(G)))
        for node in S:
            activated[node] = True
        T = [node for node in S]
        i = 0
        while i < len(T):
            v = T[i]
            for neighbor in G[v]:
                if not activated[neighbor]:
                    prob = float(P.loc[v, neighbor])
                    if random.random() < prob:
                        activated[neighbor] = True
                        T.append(neighbor)
            i += 1
        spread += len(T)
    return spread/I

def greedy_beam(G, B, Q, Ef, S, Phi, K, I, BEAM_WIDTH=3):
    """
    Return best features to PIMUS problem using greedy algorithm.
    :param G: networkx graph
    :param B: dataframe of base probabilities
    :param Q: dataframe of product probabilities
    :param Ef: dictionary feature -> edges
    :param S: list of seed set
    :param Phi: set of all features
    :param K: integer of number of required features
    :param I: integer number of Monte-Carlo simulations
    :param BEAM_WIDTH: beam width
    :return: F: list of K best features
    """
    print('Beam width: ', BEAM_WIDTH)
    P = B.copy()
    F = []
    best_beam = []
    candidates = [] # list of pairs (F, val) for choosing the best 3

    # Since py heaps do not support max heaps, we use negative spread values to order the candidates
    influence = dict()
    t = 0
    max_spread = +1 # Negative values
    print '|F| = {}'.format(len(F))
    P = B.copy()
    for f in Phi.difference(F):
        changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P) 
        spread = calculate_MC_spread(G, S, P, I)
        # if spread > max_spread:
        #     max_spread = spread
        #     max_feature = f
        decrease_probabilities(changed, P) # get back to old prob
        heapq.heappush(candidates, ((-1*spread, F.append(f))))
# got all candidates now take best beam
    best_beam = copy(candidates[:BEAM_WIDTH])
    t+=1

    while t < K:
        candidates = []
        for val, F in best_beam:
            print '|F| = {}'.format(len(F))
            P = B.copy()
            inc_prob_set(G, B, Q, F, Ef, P) # restore prob values to having set F
            for f in Phi.difference(F):
                changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P) 
                spread = calculate_MC_spread(G, S, P, I)
                # if spread > max_spread:
                #     max_spread = spread
                #     max_feature = f
                decrease_probabilities(changed, P) # get back to old prob
                heapq.heappush(candidates, ((-1*spread, F.append(f))))
        # got all candidates now take best beam
        best_beam = copy(candidates[:BEAM_WIDTH])
        t+=1
        # Now we have BEAM_WIDTH top elements for the next round
    
    negSpread, F = best_beam[0]    
    influence = -negSpread
    return F, influence

# F = greedy(G, B, Q, Ef, S, K, theta) # can be also greedy, top-edges, top-nodes, etc.

def greedy(G, B, Q, Ef, S, Phi, K, I):
    """
    Return best features to PIMUS problem using greedy algorithm.
    :param G: networkx graph
    :param B: dataframe of base probabilities
    :param Q: dataframe of product probabilities
    :param Ef: dictionary feature -> edges
    :param S: list of seed set
    :param Phi: set of all features
    :param K: integer of number of required features
    :param I: integer number of Monte-Carlo simulations
    :return: F: list of K best features
    """
    P = B.copy()
    F = []
    influence = dict()
    while len(F) < K:
        max_spread = -1
        print('|F| = {}'.format(len(F)))
        for f in Phi.difference(F):
            changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P)
            spread = calculate_MC_spread(G, S, P, I)
            if spread > max_spread:
                max_spread = spread
                max_feature = f
            decrease_probabilities(changed, P)
        F.append(max_feature)
        influence[len(F)] = max_spread
        increase_probabilities(G, B, Q, F + [max_feature], Ef[max_feature], P)
    return F, influence

def explore(G, P, S, theta):
    """
    Creates in-arborescences for nodes reachable from S.
    :param G: networkx graph
    :param P: dataframe of edge probabilities
    :param S: list seed set
    :param theta: float parameter controlling size of arborescence
    :return:
    """

    Ain = dict()
    for v in S:
        MIPs = {v: []} # shortest paths of edges to nodes from v
        crossing_edges = set([out_edge for out_edge in G.out_edges([v]) if out_edge[1] not in S + [v]])
        edge_weights = dict()
        dist = {v: 0} # shortest paths from the root v

        while crossing_edges:
            # Dijkstra's greedy criteria
            min_dist = float("Inf")
            sorted_crossing_edges = sorted(crossing_edges) # to break ties consistently
            for edge in sorted_crossing_edges:
                if edge not in edge_weights:
                    edge_weights[edge] = -math.log(float(P.loc[edge]))
                edge_weight = edge_weights[edge]
                if dist[edge[0]] + edge_weight < min_dist:
                    min_dist = dist[edge[0]] + edge_weight
                    min_edge = edge
            # check stopping criteria
            if min_dist < -math.log(theta):
                dist[min_edge[1]] = min_dist
                MIPs[min_edge[1]] = MIPs[min_edge[0]] + [min_edge]
                # update crossing edges
                crossing_edges.difference_update(G.in_edges(min_edge[1]))
                crossing_edges.update([out_edge for out_edge in G.out_edges(min_edge[1])
                                       if (out_edge[1] not in MIPs) and (out_edge[1] not in S)])
            else:
                break
        for u in MIPs:
            if u not in S:
                if u in Ain:
                    Ain[u].add_edges_from(MIPs[u])
                else:
                    Ain[u] = nx.DiGraph()
                    Ain[u].add_edges_from(MIPs[u])
    return Ain

def calculate_ap(u, Ain_v, S, P):
    """
    Calculate activation probability of u in in-arborescence of v.
    :param u: node in networkx graph
    :param Ain_v: networkx graph
    :param S: list of seed set
    :param P: dataframe of edge probabilities
    :return: float of activation probability
    """
    if u in S:
        return 1
    elif not Ain_v.in_edges(u):
        return 0
    else:
        prod = 1
        for e in Ain_v.in_edges(u):
            w = e[0]
            ap_w = calculate_ap(w, Ain_v, S, P)
            prod *= (1 - ap_w*float(P.loc[e]))
        return 1 - prod

def update(Ain, S, P):
    """
    Returns influence spread in IC model from S using activation probabilities in in-arborescences.
    :param Ain: dictionary of node -> networkx in-arborescence
    :param S: list of seed set
    :param P: dataframe of edge probabilities
    :return:
    """
    return sum([calculate_ap(u, Ain[u], S, P) for u in Ain])

def get_pi(G, Ain, S):
    """
    Get participating edges.
    :param G: networkx graph
    :param Ain: in-arborescences
    :param S: Seed set
    :return:
    """
    Pi_nodes = set(Ain.keys() + S)
    Pi = set()
    for u in Pi_nodes:
        Pi.update(G.in_edges(u))
        Pi.update(G.out_edges(u))
    return Pi

def explore_update_beam(G, B, Q, S, K, Ef, theta, BEAM_WIDTH=3):
    """
    Explore-Update algorithm which uses beam search.
    :param G: networkx graph
    :param B: dataframe base probabilities
    :param Q: dataframe product probabilities
    :param S: list of seed set
    :param K: integer of number of required features
    :param Ef: dictionary of feature to edges
    :param theta: float threshold parameter
    :return F: list of selected features
    """

    P = B.copy() # initialize edge probabilities

    Ain = explore(G, P, S, theta)
    Pi = get_pi(G, Ain, S)

    F = []
    Phi = set(Ef.keys())

    best_beam = [] # script F
    candidates = []

    t = 1
    count = 0 # What is count?

    for f in Phi.difference(F):
        e_intersection = Pi.intersection(Ef[f])
        if e_intersection:
            changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P)
            Ain = explore(G, P, S, theta)
            spread = update(Ain, S, P)
            decrease_probabilities(changed, P)
            candidates.append((-1*spread, F+[f]))
        else:
            count += 1
    
    
    heapq.heapify(candidates)

    best_beam = copy(candidates[:BEAM_WIDTH])
        
    while t < K:
        candidates = []
        print('|F| = {}'.format(t))
        for val, F in best_beam:
            P = B.copy()
            print F
            inc_prob_set(G, B, Q, F, Ef, P) # Restore prob values to these
            for f in Phi.difference(F):
                e_intersection = Pi.intersection(Ef[f])
                if e_intersection:
                    changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P)
                    Ain = explore(G, P, S, theta)
                    spread = update(Ain, S, P)
                    heapq.heappush(candidates, (-1 * spread, F + [f]))
                    decrease_probabilities(changed, P)
                else:
                    count += 1    
            best_beam = copy(candidates[:BEAM_WIDTH])
        t+=1
    F = best_beam[0][1]
    print(best_beam)
    return F



def explore_update(G, B, Q, S, K, Ef, theta):
    """
    Explore-Update algorithm.
    :param G: networkx graph
    :param B: dataframe base probabilities
    :param Q: dataframe product probabilities
    :param S: list of seed set
    :param K: integer of number of required features
    :param Ef: dictionary of feature to edges
    :param theta: float threshold parameter
    :return F: list of selected features
    """
    P = B.copy() # initialize edge probabilities

    Ain = explore(G, P, S, theta)
    Pi = get_pi(G, Ain, S)

    F = []
    Phi = set(Ef.keys())

    count = 0
    while len(F) < K:
        print('|F| = {}'.format(len(F)))
        max_feature = None
        max_spread = -1
        for f in Phi.difference(F):
            e_intersection = Pi.intersection(Ef[f])
            if e_intersection:
                changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P)
                Ain = explore(G, P, S, theta)
                spread = update(Ain, S, P)
                if spread > max_spread:
                    # print(spread)
                    max_spread = spread
                    max_feature = f
                decrease_probabilities(changed, P)
            else:
                count += 1
        if max_feature:
            F.append(max_feature)
            increase_probabilities(G, B, Q, F, Ef[max_feature], P)
            Ain = explore(G, P, S, theta)
            Pi = get_pi(G, Ain, S)
        else:
            #raise ValueError, 'Not found max_feature. F: {}'.format(F)
            raise Exception(ValueError, 'Not found max_feature. F: {}'.format(F))
    print max_spread
    return F

def calculate_spread(G, B, Q, S, F, Ef, I):
    """
    Calculate spread for given feature set F.
    :param G: networkx graph
    :param B: dataframe base probabilities
    :param Q: dataframe product probabilities
    :param S: list of seed set
    :param F: list of selected features
    :param Ef: dictionary of feature to edges
    :param I: integer number of MC calculations
    :return: float average number of influenced nodes
    """
    P = B.copy()
    E = []
    for f in F:
        E.extend(Ef[f])
    increase_probabilities(G, B, Q, F, E, P)

    return calculate_MC_spread(G, S, P, I)


def brute_force(G, B, Q, S, K, Ef, I):
    """ Return optimal solution to PIMUS problem by checking all possible combinations.
    :param G: networkx graph
    :param B: dataframe base probabilities
    :param Q: dataframe product probabilities
    :param S: list of seed set
    :param K: number of required features
    :param Ef: dictionary of feature to edges
    :param I: integer number of MC calculations
    :return: max_F list of optimal features; max_spread integer of spread achieved
    """
    Phi = set(Ef.keys())
    combs = combinations(Phi, K)
    max_spread = -1
    max_F = []
    for i, f_set in enumerate(combs):
        spread = calculate_spread(G, B, Q, S, f_set, Ef, I)
        if spread > max_spread:
            max_F = f_set
            max_spread = spread
    return max_F, max_spread

def top_edges(Ef, K):
    """ Return features based on the edges.
    :param Ef: dictionary of feature to edges
    :param K: number of required features
    :return: list of Top-Edges features
    """
    #return map(lambda (k, v): k, sorted(Ef.items(), key= lambda (k, v): len(v), reverse=True)[:K])
    return map(lambda k, v: k, sorted(Ef.items(), key= lambda k, v: len(v), reverse=True)[:K])

def top_nodes(Nf, K):
    """ Return features based on the edges.
    :param Nf: dictionary of node to features
    :param K: number of required features
    :return: list of Top-Nodes features
    """
    #return map(lambda (k, v): k, Counter(chain.from_iterable(Nf.values())).most_common(K))
    return map(lambda k, v: k, Counter(chain.from_iterable(Nf.values())).most_common(K))

def run_gnutella():

    model = "mv"
    G = read_graph('datasets/gnutella/gnutella.txt')
    Ef, Nf = add_graph_attributes(G, 'datasets/gnutella/gnutella_mem.txt')
    Phi = set(Ef.keys())
    B = read_probabilities('datasets/gnutella/gnutella_{}.txt'.format(model))
    Q = read_probabilities('datasets/gnutella/gnutella_{}.txt'.format(model))
    groups = read_groups('datasets/gnutella/gnutella_com.txt')


    S = groups['9'] # select some group as a seed set
    K = 10
    theta = 1./40
    I = 1000 # number of Monte-Carlo simulations

    print('Selecting features')
    start = time.time()
    # F = greedy(G, B, Q, S, K, Ef, theta) # can be also greedy, top-edges, top-nodes, etc.
    F = greedy_beam(G, B, Q, Ef, S, Phi, K, I, BEAM_WIDTH=3)
    finish = time.time()
    print('Selected F:', F)
    print('Time:', finish - start)

    
    spread = calculate_spread(G, B, Q, S, F, Ef, I)
    print('Spread:', spread)

    console = []

def run_vk():

    # model = "mv"
    model = "wc"

    #G = read_graph('datasets/gnutella.txt')
    # 
    # G = read_graph('../datasets/vk/vk.txt')
    
    G = read_graph('../datasets/vk/vk.txt')

    #Ef, Nf = add_graph_attributes(G, 'datasets/gnutella_mem.txt')
    Ef, Nf = add_graph_attributes(G, '../datasets/vk/vk_mem.txt')

    Phi = set(Ef.keys())

    #B = read_probabilities('datasets/gnutella_{}.txt'.format(model))
    B = read_probabilities('../datasets/vk/vk_{}.txt'.format(model))
    #Q = read_probabilities('datasets/gnutella_{}.txt'.format(model))
    Q = read_probabilities('../datasets/vk/vk_{}.txt'.format(model))

    #groups = read_groups('datasets/gnutella_com.txt')
    groups = read_groups('../datasets/vk/vk_com.txt')

    # VK PARAMETERS - from setup.txt file
    # print(type(groups), groups.keys())
    S = groups['223212']

    print ("Seed set size: ", len(S))
    
    K = 51    
    theta = 1./40
    I = 10000

    print('Selecting features')
    start = time.time()
    # F = greedy(G, B, Q, S, K, Ef, theta) # can be also greedy, top-edges, top-nodes, etc.
    F = explore_update_beam(G, B, Q, S, K, Ef, theta, BEAM_WIDTH=3)
    finish = time.time()
    print('Selected F:', F)
    print('Time:', finish - start)

    
    spread = calculate_spread(G, B, Q, S, F, Ef, I)
    print('Spread:', spread)

    console = []

def run_vk_small():
    model = "wc"

    #G = read_graph('datasets/gnutella.txt')
    # 
    # G = read_graph('../datasets/vk/vk.txt')
    
    G = read_graph('../datasets/vk/vk_small.txt')

    #Ef, Nf = add_graph_attributes(G, 'datasets/gnutella_mem.txt')
    Ef, Nf = add_graph_attributes(G, '../datasets/vk/vk_mem.txt')

    Phi = set(Ef.keys())

    #B = read_probabilities('datasets/gnutella_{}.txt'.format(model))
    B = read_probabilities('../datasets/vk/vk_{}.txt'.format(model))
    #Q = read_probabilities('datasets/gnutella_{}.txt'.format(model))
    Q = read_probabilities('../datasets/vk/vk_{}.txt'.format(model))

    #groups = read_groups('datasets/gnutella_com.txt')
    groups = read_groups('../datasets/vk/vk_com.txt')

    # GNUTELLA PARAMETERS
    # S = groups['9'] # select some group as a seed set
    # K = 10
    # theta = 1./40
    # I = 1000 # number of Monte-Carlo simulations


    # VK PARAMETERS
    # print(type(groups), groups.keys())
    S = groups['31598870']
    K = 51    
    theta = 1./40
    I = 10000

    print('Selecting features')
    start = time.time()
    # F = greedy(G, B, Q, S, K, Ef, theta) # can be also greedy, top-edges, top-nodes, etc.
    F = greedy_beam(G, B, Q, Ef, S, Phi, K, I, BEAM_WIDTH=3)
    finish = time.time()
    print('Selected F:', F)
    print('Time:', finish - start)

    
    spread = calculate_spread(G, B, Q, S, F, Ef, I)
    print('Spread:', spread)

    console = []
    
def run_toy():
    
    G = read_graph('../datasets/toy/edge_list.txt')
    Ef, Nf = add_graph_attributes(G, '../datasets/toy/mem.txt')

    Phi = set(Ef.keys())

    B = read_probabilities('../datasets/toy/edge_weights.txt')
    Q = read_probabilities('../datasets/toy/edge_weights.txt')


    groups = read_groups('../datasets/toy/com.txt')
 
    # PARAMETERS
    # print(type(groups), groups.keys())
    S = groups['39545549']
    print(S)
    K = 3    
    theta = 1./40
    I = 10000

    print('Selecting features')
    start = time.time()
    # F = greedy(G, B, Q, S, K, Ef, theta) # can be also greedy, top-edges, top-nodes, etc.
    # F, _ = greedy_beam(G, B, Q, Ef, S, Phi, K, I)
    F = explore_update_beam(G, B, Q, S, K, Ef, theta, BEAM_WIDTH=3) # can be also greedy, top-edges, top-nodes, etc.
    # F = greedy_beam(G, B, Q, Ef, S, Phi, K, I, BEAM_WIDTH=3)
    finish = time.time()
    print('Selected F:', F)
    print('Time:', finish - start)

    
    spread = calculate_spread(G, B, Q, S, F, Ef, I)
    print('Spread:', spread)

    console = []

def run_general(dataset, K, SG, algo='eu', theta=1./40,  I=10000, BEAM_WIDTH=3):
    """
    Generic function to run the experiment
    :param dataset: Dataset to be used
    :param K: Number of features to be selected 
    :param SG: Group number to be used as the seed
    :param algo: Algorithm to be used to calculate the feature set
    :param I: Number of MC simulations
    :param BEAM_WIDTH: BEAM WIDTH
    """
    if dataset == "vk":
        model = "wc"
        G = read_graph('../datasets/vk/vk.txt')
        Ef, Nf = add_graph_attributes(G, '../datasets/vk/vk_mem.txt')
        Phi = set(Ef.keys())
        B = read_probabilities('../datasets/vk/vk_{}.txt'.format(model))
        Q = read_probabilities('../datasets/vk/vk_{}.txt'.format(model))
        groups = read_groups('../datasets/vk/vk_com.txt')

    else:
        # READ GRAPH
        G = read_graph(os.path.join('../datasets/', dataset, 'edge_list.txt'))
        Ef, Nf = add_graph_attributes(G, os.path.join('../datasets/', dataset, 'mem.txt'))
        Phi = set(Ef.keys())
        B = read_probabilities(os.path.join('../datasets/', dataset, 'edge_weights.txt'))
        Q = read_probabilities (os.path.join('../datasets/', dataset, 'edge_weights.txt'))
        groups = read_groups(os.path.join('../datasets/', dataset, 'com.txt'))

    S = groups[str(SG)]
    print(S)
    
    print('Selecting features')
    start = time.time()

    if algo=='g':
        F, _ = greedy(G, B, Q, S, K, Ef, theta) 
    
    if algo=='gb':
        F, _ = greedy_beam(G, B, Q, Ef, S, Phi, K, I) 
    
    if algo=='eu':
        F = explore_update(G, B, Q, S, K, Ef, theta) 
    
    if algo=='eub':
        F = explore_update_beam(G, B, Q, S, K, Ef, theta, BEAM_WIDTH) 
    finish = time.time()
    print('Selected F:', F)
    print('Time:', finish - start)

    spread = calculate_spread(G, B, Q, S, F, Ef, I)
    print('Spread:', spread)

    console = []

if __name__ == "__main__":
    args = parser.parse_args()

    if args.dataset == 'vk':
        SG = 223212
    elif args.dataset == 'toy':
        SG = 39545549
    elif args.dataset == 'medium':
        SG = 39545549
    else: # GNUTELLA?        
        SG = 9
    
    if args.SG:
        SG = args.sg
    
    run_general(dataset=args.dataset, K=args.K, SG=SG, algo=args.algo, theta=args.theta, I=args.I, BEAM_WIDTH=args.bw)

    # run_vk()