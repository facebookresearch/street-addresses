# Copyright (c) 2017-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
#

""" Runs complete pipeline for robocode generation """

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals


import argparse
import logging
from os import makedirs
from os.path import exists
import sys
from util import generate_osm_rtree
from util import safal_functions
from util import osm2geotiff


ALLOWED_EXTENSIONS_INPUT = set(['tif', 'tiff'])


def allowed_file_input(filename):
    """
    Checks if input file name is of allowed type, accepts only geotiffs

    :param filename: filename with full extension
    :return: boolean
    """
    return '.' in filename and filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS_INPUT


def create_logger(args):
    """
    Creating logger object for logging processes

    :param args: arguments to log
    :return: logger object
    """
    logger = logging.getLogger('stdout')
    hdlr = logging.FileHandler(args['out_dir'] + '/' + 'stdout.log')
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


def osm_rtree_generator(ntr_json, gps, out_dir, logger):
    """
    Generates OSM and Rtree data

    :param ntr_json: json that contains name to roads info
    :param gps: lat and lon bounding box info of input
    :param out_dir: directory to save results
    :param logger: logger object for logging
    """
    logger.info('Generating Roads OSM file')
    generate_osm_rtree.main(ntr_json, out_dir,
                            gps[0], gps[1], gps[2], gps[3], logger)


def main(args, out_fn, logger):
    """
    :param args: input arguments
    :param out_fn: absolute path of file to be processed
    :param logger: logger object for logging

    """
    logger.info('Processing file:' + out_fn)
    out_dir = args['out_dir']
    if not exists(out_dir):
        makedirs(out_dir)
    # Creating a safal object
    safal_layers = safal_functions.SAFAL(args, out_fn, out_dir, logger)

    # Road segmenter initiated
    logger.info('Starting Road segmentator')
    safal_layers.RoadSegment()

    # Region creator initiated
    logger.info('Starting Region Creator')
    ntr_json = safal_layers.RegionProcess()

    # Get bounding box info
    gps = safal_layers.cal_gps()
    osm_rtree_generator(ntr_json, gps, out_dir, logger)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--input_tiff', default=None, type=str,
        help='Path to input geotiff tile')
    parser.add_argument(
        '--xml', default=None, type=str,
        help='Path to input Osm file')
    parser.add_argument(
        '--out_dir', required=True, type=str,
        help='Output dir where all results will be written')
    parser.add_argument(
        '--roadSeg_bin', required=True, type=str,
        help='Binary for road segmentation')
    parser.add_argument(
        '--centre_row', required=False, type=int,
        help='Row dimension of city center')
    parser.add_argument(
        '--centre_col', required=False, type=int,
        help='Column dimension of city center')
    args = vars(parser.parse_args())

    # getting logger object
    logger = create_logger(args)

    # Checking for geotiff input
    if args['input_tiff'] is not None:
        filename = args['input_tiff'].split('/')[-1]
        logger.info('Reading Road Imagery Geotiff')
        if allowed_file_input(filename) is not True:
            logger.error('Invalid input_file! accepts only tiff')
            sys.exit(-1)
        out_fn = args['out_dir'] + '/' + filename
        logger.info('Running end2end with roads geotiff')
        main(args, out_fn, logger)
    # Checking for OSM input
    elif args['xml'] is not None:
        filename = args['xml'].split('/')[-1].split('.')[0]
        logger.info('Reading OSM file')
        filepath = args['out_dir'] + '/' + filename + '.tif'
        # Converting OSM to geotiff image
        osm2geotiff.main(args['xml'], filepath)
        out_fn = filepath
        logger.info('Running end2end with OSM as input')
        main(args, out_fn, logger)
    else:
        logger.info('Give a valid input!')
        sys.exit(-1)
