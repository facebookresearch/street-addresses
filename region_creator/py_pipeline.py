# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

""" Main script that starts region generation """

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals


import argparse
import json
import logging
from region_creator import create_regions
from region_creator import name_regions
from region_creator import change_names_ends
import sys
sys.path.insert(0,'../')


def create_logger(args):
    """
    Creating logger object for logging processes

    :param args: arguments to log
    :return: logger object
    """
    logger = logging.getLogger('regionCreator')
    hdlr = logging.FileHandler(args['out_dir'] + '/' + 'regionCreator.log')
    msg_format = '%(asctime)s [%(levelname)s] %(message)s'
    formatter = logging.Formatter(msg_format)
    hdlr.setFormatter(formatter)
    streamHandler = logging.StreamHandler()
    streamHandler.setFormatter(formatter)
    logger.addHandler(hdlr)
    logger.addHandler(streamHandler)
    logger.setLevel(logging.DEBUG)
    logger.info(sys.version_info)
    logger.info(args)
    return logger


def main(js_fn, o_dir, logger, center_r=None, center_c=None, c_mask=None):
    """
    Main function that starts region creator

    :param js_fn: path to .json file
    :param o_dir: output dir for writing results
    :param logger: logger object for logging
    :param center_r: row coordinate of center of city
    :param center_c: column coordinate of center of city
    :param c_mask: Mask to restrict region growing algorithm
    :return: json that contains name to road info
    """
    # Reading json
    json_s = open(js_fn).read()
    json_d = json.loads(json_s)

    id_to_road_m = json_d['id_road']
    pixel_to_id_m = json_d['pixel_road']
    row_m, col_m = json_d['img_meta']['height'], json_d['img_meta']['width']

    logger.info('Beginning create_regions.py')
    inter_to_color, color_to_mean = create_regions.main(
        id_to_road_m, pixel_to_id_m, o_dir, c_mask
    )

    logger.info('Beginning name_regions.py')
    color_to_name = name_regions.main(
        row_m, col_m, inter_to_color, color_to_mean, o_dir,
        center_r, center_c
    )

    logger.info('Beginning change_names_ends.py')
    name_to_road = change_names_ends.main(
        inter_to_color, id_to_road_m, color_to_name, o_dir, row_m, col_m
    )

    return name_to_road


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument(
        '-json',
        '--json',
        required=True,
        help='Path to id_to_road/pixel_to_id (json)'
    )
    ap.add_argument(
        '-c_r',
        '--center_row',
        required=False,
        help='Row dimension of city center'
    )
    ap.add_argument(
        '-c_c',
        '--center_col',
        required=False,
        help='Column dimension of city center'
    )
    ap.add_argument(
        '-o_dir', '--out_dir', required=True, help='Directory to save output'
    )
    args = vars(ap.parse_args())

    o_dir = args['out_dir']
    js_fn = args['json']
    center_r = args['center_row']
    center_c = args['center_col']
    logger = create_logger(args)
    main(js_fn, o_dir, logger, center_r, center_c)
