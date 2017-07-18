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
import math


class Region_Namer:
    """ regions named according to AB, where A in (N,S,E,W), B in (A - Z) """

    def __init__(self, center_r, center_c):
        self.color_to_name = {}  # maps a color(index) to a name
        self.center_r = int(center_r / 10)
        self.center_c = int(center_c / 10)

    def name(self, inter_to_color, color_to_mean):
        """ names regions of mask image based on centroid orientation from
            the center

            Keyword arguments:
            mask -- region image
        """
        first = 1
        cardinal = ["N", "S", "E", "W"]
        unused_names = [
            [
                card + a
                for a in map(chr, range(65, 91)) if a not in map(chr, [73, 79])
            ] for card in cardinal
        ]

        # sort the regions according to distance from center
        for key, val in sorted(
            color_to_mean.items(),
            key=lambda t: dist((self.center_r, self.center_c), t[1])
        ):
            x, y = int(val[0]), int(val[1])
            # find the correct bucket for the current region
            card = nsew(x, y, self.center_r, self.center_c)
            if first:
                name = "CA"  # closest region to center is always named CA
                first = 0
            else:
                try:
                    name = unused_names[card].pop(0)
                except IndexError as error:
                    print("You've exceeded the number of possible regions.")
                    print("Try smaller area.")
                    raise
            self.color_to_name[key] = name


def dist(p1, p2):
    """ euclidean dist of two points

        Keyword arguments:
        p1 -- x, y point in pixel space
        p2 -- x, y point in pixel space
    """
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)


def nsew(x, y, center_r, center_c):
    """ return orientation of centroid from city center

        Keyword arguments:
        x -- x of x, y point in pixel space
        y -- y of x, y point in pixel space
        center_r -- center of city row
        center_c -- center of city col
    """
    angle = (angle_between((center_r, center_c), (x, y)) + 180 + 45) % 360

    if 0 <= angle < 90:
        return 0
    if 90 <= angle < 180:
        return 3
    if 180 <= angle < 270:
        return 1
    if 270 <= angle < 360:
        return 2


def angle_between(p1, p2):
    """ angle between two points

        Keyword arguments:
        p1 -- x, y point
        p2 -- x, y point
    """
    p2 = list(p2)
    p2[0] = p2[0] - p1[0]
    p2[1] = (p2[1] - p1[1])
    angle = math.degrees(math.atan2(p2[1], p2[0]))
    angle = 360 + angle if angle < 0 else angle
    return angle


def get_angle(mp, p1, p2):
    """ get angle formed between p1, mp, p2

        Keyword arguments:
        mp -- midpoint, x, y point
        p1 -- x, y point
        p2 -- x, y point
    """
    if mp in [p1, p2]:
        return 0
    try:
        angle = math.degrees(
            math.acos(
                (dist(mp, p1)**2 + dist(mp, p2)**2 - dist(p1, p2)**2) /
                (2 * dist(mp, p1) * dist(mp, p2))
            )
        )
    except Exception:
        return 0
    else:
        return angle


def main(
    row,
    col,
    inter_to_color,
    color_to_mean,
    o_dir,
    center_r=None,
    center_c=None
):
    if center_r is None and center_c is None:
        center_r, center_c = row // 2, col // 2

    r_namer = Region_Namer(int(center_r), int(center_c))
    r_namer.name(inter_to_color, color_to_mean)

    return r_namer.color_to_name
