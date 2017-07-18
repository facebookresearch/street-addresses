# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import itertools
import numpy as np
import random
from scipy import sparse
from sklearn.cluster import SpectralClustering


def create_graph_inverse(id_to_road, pixel_to_id):
    """ returns graph representation of roads as adjacency matrix

        Creates an undirected weighted graph where nodes are intersections and
        edges are the segment of road which lies between two intersections
        weighted by the distance between the two intersections.
        Constructs supernodes by merging nearby interesections.
        When looking for intersections, we follow a single road and look in a
        2x2 window around the pixel for any other roads in the window.
        The window is a safeguard against T junctions where one road is separated
        by a single pixel from the road it forms a junction with.

        Keyword arguments:
        r -- rows in pixel image
        c -- columns in pixel image
        id_to_road -- dict which maps road ids to the pixels in the road
        pixel_to_id -- dict which maps all road pixels to their ids
    """

    seen_inter = {}  # maps a road to a set of roads that it intersects
    adj_list = {}  # list of edges for a particular intersection
    inter_to_id = {}  # maps the intersection to a unique node id
    counter = itertools.count(start=0, step=1)

    for curr_id in id_to_road.keys():
        seen_inter[int(curr_id)] = set()

    for curr_id, curr_road in id_to_road.items():
        prev = (curr_road[0][0], curr_road[0][1])
        if prev not in adj_list:
            adj_list[prev] = set()
        if prev not in inter_to_id:
            count = next(counter)
            inter_to_id[prev] = count

        for pixel in curr_road:
            for i in range(-2, 3):
                for j in range(-2, 3):
                    new_id = pixel_to_id.get(
                        "(" + str(pixel[0] + i) + ", " + str(pixel[1] + j) + ")"
                    )

                    if new_id:
                        new_id = int(new_id)

                    if new_id not in [None, int(curr_id)]:
                        seen_inter[int(curr_id)].add(int(new_id))
                        seen_inter[int(new_id)].add(int(curr_id))
                        if (pixel[0], pixel[1]) not in adj_list:
                            adj_list[(pixel[0], pixel[1])] = set()
                        if prev not in inter_to_id:
                            count = next(counter)
                            inter_to_id[(pixel[0], pixel[1])] = count

                        adj_list[(pixel[0], pixel[1])].add(prev)
                        adj_list[prev].add((pixel[0], pixel[1]))
                        prev = (pixel[0], pixel[1])
                        if prev not in inter_to_id:
                            count = next(counter)
                            inter_to_id[prev] = count

    del seen_inter
    blacklist = create_supernodes(adj_list, inter_to_id)
    return convert_to_adj_mat(inter_to_id, adj_list, blacklist)


def create_supernodes(adj_list, inter_to_id):
    """ create supernodes, placing absorbed nodes into blacklist

        Keyword arguments:
        adj_list -- dict which maps a graph node index to a set of other indices
        inter_to_id -- dict which maps intersection to its index in adj list

        Returns:
        set of points to remove from graph
    """
    blacklist = set()  # list for removal of unwanted nodes (create supernodes)
    for node1 in adj_list.keys():
        for node2 in adj_list.keys():
            if node1 != node2 and adj_list[node1] and adj_list[node2]:
                if ((node1[0] - node2[0])**2 +
                    (node1[1] - node2[1])**2)**.5 < 7:
                    adj_list[node1].update(adj_list[node2])
                    if node2 in adj_list[node1]:
                        adj_list[node1].remove(node2)
                    blacklist.add(inter_to_id[node2])
                    for node3 in list(adj_list[node2]):
                        if node2 in adj_list[node3]:
                            adj_list[node3].remove(node2)
                        adj_list[node3].add(node1)
                    adj_list[node2] = set()
    return blacklist


def convert_to_adj_mat(inter_to_id, adj_list, blacklist):
    """ converts adjacency list to adjacency matrix and returns adjacency mat

        converts adjacency list to adjacency matrix while removing points which
        are in the blacklist -- adjusts the indices (ids) of vertices so that
        the matrix has no disconnected components

        Keyword arguments:
        inter_to_id -- maps an intersection (x, y) to a unique id to label nodes
        adj_list -- adjacency list of graph (see comment in create_graph_inverse)
        blacklist -- set of nodes to remove from graph
    """
    dim = len(adj_list)
    adj_mat = sparse.dok_matrix((dim, dim))

    # initial population of adj_mat
    for node, edges in adj_list.items():
        prev = inter_to_id[node]
        for edge in edges:
            curr = inter_to_id[edge]
            dist = ((node[0] - edge[0])**2 + (node[1] - edge[1])**2)**.5
            adj_mat[prev, curr] = dist
            adj_mat[curr, prev] = dist

    # only create adj mat for largest connected comp so clustering converges
    connected_comps = sparse.csgraph.connected_components(adj_mat)[1]
    hist = np.histogram(
        connected_comps, bins=np.arange(max(connected_comps))
    )[0]
    max_label = np.argmax(hist)
    del hist
    num_removed = 0  # used to keep track of the number of nodes removed to adjust
    # the number of rows/columns of the adjacency matrix
    inter_to_id2 = {}
    id_to_inter2 = {}
    # remove nodes which are not part of max connected comp or are in the
    # blacklist
    for prev_inter, prev_id in sorted(
        [(x, inter_to_id[x]) for x in adj_list.keys()], key=lambda y: y[1]
    ):
        if prev_id in blacklist or connected_comps[prev_id] != max_label:
            num_removed += 1
        else:
            new_id = prev_id - num_removed
            inter_to_id2[prev_inter] = new_id
            id_to_inter2[new_id] = prev_inter

    dim = dim - num_removed
    adj_mat = sparse.dok_matrix((dim, dim))

    # population of adj mat, only adding edges not composed of removed nodes
    for node, edges in adj_list.items():
        if node in inter_to_id2:
            prev = inter_to_id2[node]
            for edge in edges:
                curr = inter_to_id2[edge]
                dist = ((node[0] - edge[0])**2 + (node[1] - edge[1])**2)**.5
                adj_mat[prev, curr] = dist
                adj_mat[curr, prev] = dist

    return adj_mat, id_to_inter2


def color_graph(comps, id_to_road, id_to_inter, pixel_to_id):
    """ colors graph based on intersection labels (comps)

        colors each node graph and returns mapping of intersection to color given
        along with dict that maps a color (region) to its average pixel

        Keyword arguments:
        comps -- list of cluster labels for each graph node
        id_to_road -- dict which maps road id to all points in the road
        id_to_inter -- dict which maps an intersection key to the (x, y) value
        pixel_to_id -- dict which maps (x, y) value to its intersection id

        Returns:
        inter_to_color dict which maps (x, y) to color (label) and color_to_mean
        dict which maps label to centroid (x, y) point
    """
    colors = set((0, 0, 0))
    id_to_color = {}
    inter_to_color = {}
    color_to_points = {}
    color_to_mean = {}

    for value in set(comps):
        color = (
            random.randint(0, 255), random.randint(0, 255),
            random.randint(0, 255)
        )
        while color in colors:
            color = (
                random.randint(0, 255), random.randint(0, 255),
                random.randint(0, 255)
            )
        colors.add(color)
        id_to_color[value] = color
        color_to_points[color] = []

    for inter_id, color_key in enumerate(comps):
        point = id_to_inter.get(inter_id)
        if point:
            xn, yn = point
            color = id_to_color[color_key]
            color_to_points[color].append(point)
            inter_to_color[point] = color

    for key, value in color_to_points.items():
        color_to_mean[key] = np.mean(value, axis=0) / 10

    return inter_to_color, color_to_mean


def main(id_to_road, pixel_to_id, o_dir, c_mask=None):
    coms = []
    adj_mat, id_to_inter = create_graph_inverse(id_to_road, pixel_to_id)
    k = int(len(id_to_road.keys()) // 88) + 1

    affinity = 'precomputed'

    coms = SpectralClustering(
        n_clusters=k,
        eigen_solver=None,
        random_state=None,
        n_init=500,
        gamma=1,
        affinity=affinity,
        n_neighbors=5,
        eigen_tol=0,
        assign_labels='discretize',
        degree=3,
        coef0=1,
        kernel_params=None
    ).fit_predict(adj_mat)

    return color_graph(coms, id_to_road, id_to_inter, pixel_to_id)
