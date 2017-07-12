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
import numpy as np
import json
import math
from scipy.spatial import distance
import sys
sys.path.insert(0,'../')
from regionCreator import compress_json


class Name_Changer:
    """ changes roads from unique road id to ABXY where AB is the region and XY
        is an integer
    """

    def __init__(self):
        self.name_to_road = {}

    def give_names(self, id_to_road, color_to_name, inter_to_color):
        """ populates the name_to_road json mapping road name to ordered pixels
            in the road

            buckets the roads according to the label of their starting
            intersection (from create_regions). orders roads by finding
            two most prominent axis and ordering based on the average pixel
            of each road along that axis for that particular region

            Keyword arguments:
            id_to_road -- dict which maps road id to ordered pixels in road
            color_to_name -- maps a color (region index) to name of region
            inter_to_color -- dict which maps road intersection to its color
        """
        color_to_rids = {}
        orients = 8
        o_angle = 180 / float(orients)

        # assign colors two character names
        for color in color_to_name.keys():
            color_to_rids[color] = []

        # determine which color/region a road belongs to
        for rid, road in id_to_road.items():
            x, y, d = road[0]
            for i in range(x - 2, x + 3):
                for j in range(x - 2, x + 3):
                    if (i, j) in inter_to_color:
                        color = inter_to_color[(i, j)]
            else:
                color = min(
                    inter_to_color.items(),
                    key=lambda t: distance.euclidean(t[0], (x, y))
                )[1]
            # average road pixel is used for ordering roads
            avg_pix = np.average(road, axis=0)
            color_to_rids[color].append([rid, avg_pix])

        index = 0
        for color, roads in color_to_rids.items():
            index += 1
            gradients = [0 for i in range(orients)]
            even_odd = [[], []]

            # determine the directionality of the roads in a region
            for road in roads:
                angle = get_angle(road[0], id_to_road)
                if angle:
                    rid = road[0]
                    avg_pix = road[1]
                    gradients[int(round(angle / float(o_angle))) % orients] += 1

            sort_grad_ind = np.argsort(gradients)
            axis = [sort_grad_ind[-1], sort_grad_ind[-2]]
            sort_grad_ind = list(sort_grad_ind[:-2])
            while mod_dist(axis[0], axis[1], orients) <= 1:
                axis[1] = sort_grad_ind.pop(-1)

            if (2 <= axis[0] <= 5):
                axis[0], axis[1] = axis[1], axis[0]
            # append roads to even_odd based on which direction they are closer
            for road in roads:
                angle = get_angle(road[0], id_to_road)
                if angle:
                    angle = angle / float(o_angle)
                    rid = road[0]
                    avg_pix = road[1]
                    if mod_dist(angle, axis[0], orients) < \
                       mod_dist(angle, axis[1], orients):
                        even_odd[0].append((rid, avg_pix))
                    else:
                        even_odd[1].append((rid, avg_pix))

            # for each axis order the roads and append to name_to_road
            for ind in range(2):
                i = 10 + ind
                rot_points = []
                for rid, _avg_pix in sorted(
                    even_odd[ind],
                    key=lambda x: rotate(x[1], axis[ind] * o_angle)[0],
                    reverse=True
                ):
                    name = str(color_to_name.get(color)) + str(i)
                    self.name_to_road[str(name)] = id_to_road[rid]

                    for point in id_to_road[rid]:
                        rot_p = rotate(point, axis[ind] * o_angle)
                        rot_points.append(rot_p)

                    i += 2


def mod_dist(a, b, n):
    """ returns 'clock hand distance' between a, b, mod n

        Keyword arguments:
        a -- number to find distance between
        b -- number to find distance between
        n -- number to modulo by
    """

    return min((a - b) % n, (b - a) % n)


def rotate(point, theta):
    """ rotate a point treated as vector from origin by theta degrees

        Keyword arguments:
        point -- x, y point treated as 2d vector
        theta -- degrees from x axis
    """
    theta = math.radians(theta)
    rot_mat = np.array(
        [[np.cos(theta), -np.sin(theta)], [np.sin(theta), np.cos(theta)]]
    )
    return np.dot(rot_mat, [point[0], point[1]])


def get_angle(rid, id_to_road):
    """ returns angle of road start and end points

        Keyword arguments:
        rid -- road id, key for id_to_road dict
        id_to_road -- dict which maps road id to ordered pixels of road
    """
    road = id_to_road[rid]
    angle = 0
    if len(road) > 4:
        start, end = road[1], road[-2]

        l = min(start, end, key=lambda p: p[0])
        m = ((start[0] + end[0]) / float(2), (start[1] + end[1]) / float(2))
        n = (m[0], m[1] + 100)

        A = distance.euclidean((l[0], l[1]), m)
        B = distance.euclidean(n, m)
        C = distance.euclidean((l[0], l[1]), n)

        if A == 0 or B == 0:
            return None
        angle = math.degrees(math.acos((A**2 + B**2 - C**2) / (2.0 * A * B)))
    return angle


def main(inter_to_color, id_to_road, color_to_name, o_dir, r, c):
    changer = Name_Changer()
    changer.give_names(id_to_road, color_to_name, inter_to_color)

    name_to_road = compress_json.main(changer.name_to_road)
    name_to_road = [{'height': r, 'width': c}, name_to_road]
    json.dump(
        name_to_road,
        open(o_dir + '/' + 'name_to_road.json', 'w')
    )
    return name_to_road
