# author: sivanov
# date: 27 Oct 2015
from __future__ import division
import networkx as nx
import pandas as pd
import time
import random
import math
from collections import Counter
from itertools import chain, combinations

def read_graph(filename, directed=True, sep=' ', header = None):
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
    print 'Read graph'
    return G

def read_graph2(filename, directed=False):
    """
    Create networkx graph reading file.
    :param filename: every line (u, v)
    :param directed: boolean
    :return:
    """
    if not directed:
        G = nx.Graph()
    else:
        G = nx.DiGraph()
    with open(filename) as f:
        for line in f:
            d = line.split()
            G.add_edge(int(d[0]), int(d[1]))
    print 'Read Graph'
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
            G.node[u]['Fu'] = features
            Nf[u] = features
    print 'Read graph attributes'
    return Ef, Nf

def read_probabilities(filename, sep=' '):
    """
    Creates a dataframe object indexed by endpoints of edge (u, v)
    :param filename: (u, v, p)
    :param sep: separator in the file
    :return:
    """
    df = pd.read_csv(filename, sep=sep, header = None)
    print 'Read probabilities'
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
    print 'Read groups'
    return groups

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
    :return:
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
    P = B.copy()
    F = []
    best_beam = []
    candidates = [] # list of pairs (F, val) for choosing the best 3
    influence = dict()
    t = 0
    while t < K:
        for F in best_beam:
            max_spread = -1 # iterative calculation of the maximum value
            print '|F| = {}'.format(len(F))
            for f in Phi.difference(F):
                changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P)
                spread = calculate_MC_spread(G, S, P, I)
                if spread > max_spread:
                    max_spread = spread
                    max_feature = f
                decrease_probabilities(changed, P)
            t += 1
            F.append(max_feature) # length always increases by 1
            influence[len(F)] = max_spread
            increase_probabilities(G, B, Q, F + [max_feature], Ef[max_feature], P)
    return F, influence


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
        print '|F| = {}'.format(len(F))
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
        print '|F| = {}'.format(len(F))
        max_feature = None
        max_spread = -1
        for f in Phi.difference(F):
            e_intersection = Pi.intersection(Ef[f])
            if e_intersection:
                changed = increase_probabilities(G, B, Q, F + [f], Ef[f], P)
                Ain = explore(G, P, S, theta)
                spread = update(Ain, S, P)
                if spread > max_spread:
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
            raise ValueError, 'Not found max_feature. F: {}'.format(F)
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
    return map(lambda (k, v): k, sorted(Ef.items(), key= lambda (k, v): len(v), reverse=True)[:K])

def top_nodes(Nf, K):
    """ Return features based on the edges.
    :param Nf: dictionary of node to features
    :param K: number of required features
    :return: list of Top-Nodes features
    """
    return map(lambda (k, v): k, Counter(chain.from_iterable(Nf.values())).most_common(K))

if __name__ == "__main__":

    model = "mv"

    G = read_graph('datasets/gnutella.txt')
    Ef, Nf = add_graph_attributes(G, 'datasets/gnutella_mem.txt')
    Phi = set(Ef.keys())

    B = read_probabilities('datasets/gnutella_{}.txt'.format(model))
    Q = read_probabilities('datasets/gnutella_{}.txt'.format(model))

    groups = read_groups('datasets/gnutella_com.txt')

    S = groups['9'] # select some group as a seed set
    K = 10
    theta = 1./40

    print 'Selecting features'
    start = time.time()
    F = explore_update(G, B, Q, S, K, Ef, theta) # can be also greedy, top-edges, top-nodes, etc.
    finish = time.time()
    print 'Selected F:', F
    print 'Time:', finish - start

    I = 1000 # number of Monte-Carlo simulations
    spread = calculate_spread(G, B, Q, S, F, Ef, I)
    print 'Spread:', spread

    console = []