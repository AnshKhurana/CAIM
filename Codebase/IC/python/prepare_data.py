# author: sivanov
# date: 06 Nov 2015
from __future__ import division
import networkx as nx
import numpy as np
import math
import graph_tool.all as gt
import time



def convert_idx(filename, output):
    """
    Convert graph file indexing to 0 1 2 ...
    :param filename:
    :param output:
    :return:
    """
    old2new = dict()
    count = 0
    with open(filename) as f:
        with open(output, 'w+') as g:
            for line in f:
                d = line.split()
                u = int(d[0])
                v = int(d[1])
                if u not in old2new:
                    old2new[u] = count
                    count += 1
                if v not in old2new:
                    old2new[v] = count
                    count += 1
                if u != v:
                    g.write('%s %s\n' %(old2new[u], old2new[v]))


def read_graph2(filename, directed=True):
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
    return G

def wc_model(G):
    P = dict()
    for u in G:
        for v in G[u]:
            d = len(G.in_edges(v))
            P[(u,v)] = 1./d
    return P

def mv_model(G, prange):
    P = dict()
    for e in G.edges():
        p = np.random.choice(prange)
        P[e] = p
    return P

def add_weights(G, P, log=True):
    for e in P:
        if log:
            G[e[0]][e[1]]['weight'] = -math.log(P[e])
        else:
            G[e[0]][e[1]]['weight'] = P[e]

def write_communities(infile, community_file, member_file, min_B, max_B, overlap=True, verbose=True):
    """
    Writes communities and members from graph file.
    :param input: graph filename
    :param community_file: community to members
    :param member_file: node to communities
    :return:
    """
    G = read_graph2(infile)
    N = len(G)

    g = gt.Graph()
    g.add_vertex(N)

    for e in G.edges():
        g.add_edge(g.vertex(e[0]), g.vertex(e[1]))

    # g = gt.collection.data["polbooks"] # sample graph to test function

    state = gt.minimize_blockmodel_dl(g, min_B=min_B, max_B = max_B, overlap=overlap, verbose=verbose)
    blocks = state.get_overlap_blocks()
    bv = blocks[0]
    Bl = dict()
    with open(member_file, 'w') as f:
        for u in g.vertices():
            print u, list(bv[u])
            f.write("{} {}\n".format(u, " ".join(map(str, list(bv[u])))))
            for block in list(bv[u]):
                Bl.setdefault(block, []).append(u)

    with open(community_file, 'w') as f:
        for block, nodes in Bl.items():
            f.write("{} {}\n".format(block, " ".join(map(str, nodes))))



if __name__ == "__main__":
    write_communities("datasets/gnutella.txt", "datasets/gnutella_com2.txt", "datasets/gnutella_mem2.txt", min_B=150, max_B=500)
    console = []