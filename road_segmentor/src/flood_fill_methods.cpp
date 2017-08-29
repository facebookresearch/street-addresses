/*
 *  Copyright (c) 2017, Facebook, Inc.
 *  All rights reserved.
 *
 *  This file contains methods that utilize the flood fill algorithm.
 *  The methods in this file are utilized in SegmentationMethods.cpp file
 */

#include "road_segmentation.h"

void roadFloodFill(Mat image, Mat marked, int row, int col, int roadNumber)
{
    // This method performs a floodFill operation to traverse a specific road
    // until we reach a junction pixel. All the road points collected while traversal
    // are stored in teh roadLabels vector.

    if (row >= image.rows || col >= image.cols || row < 0 || col < 0)
        return;

    if (image.at<uchar>(row, col) == 0)
        return;

    if (marked.at<uchar>(row, col) == 255)
        return;

    if (isJunction(image, row, col))
    {
        roadLabels[roadNumber].push_back(Point(col,row));
        marked.at<uchar>(row,col) = 255;
        return;
    }

    roadLabels[roadNumber].push_back(Point(col,row));
    marked.at<uchar>(row,col) = 255;

    roadFloodFill(image, marked, row + 1, col, roadNumber);
    roadFloodFill(image, marked, row - 1, col, roadNumber);
    roadFloodFill(image, marked, row, col + 1, roadNumber);
    roadFloodFill(image, marked, row, col - 1, roadNumber);
    roadFloodFill(image, marked, row - 1, col - 1, roadNumber);
    roadFloodFill(image, marked, row - 1, col + 1, roadNumber);
    roadFloodFill(image, marked, row + 1, col - 1, roadNumber);
    roadFloodFill(image, marked, row + 1, col + 1, roadNumber);
}

void roadFloodFillColor(Mat colorImage, Mat marked, int row, int col, int color[3], int roadNumber)
{
    // This method traverses through all the pixels of a given color and stores the pixels
    // in an ordered fashion. New pixels are appended to the list either in front or at the end
    // based on their distance from the front and end of the vector.

    if (row >= colorImage.rows || col >= colorImage.cols || row < 0 || col < 0)
        return;

    if (colorImage.at<Vec3b>(row,col) == Vec3b(0,0,0))
        return;

    if (marked.at<uchar>(row,col) == 255)
        return;

    if (colorImage.at<Vec3b>(row, col) == Vec3b(color[0], color[1], color[2]))
    {
        marked.at<uchar>(row, col) = 255;
        //roadLabels[roadNumber].push_back(Point(col,row));

        if(roadLabels[roadNumber].empty())
            roadLabels[roadNumber].push_back(Point(col, row));

        else if (roadLabels[roadNumber].size() > 0)
        {
            //if distance from beginnning is < distance from end
            if (pixelDist(roadLabels[roadNumber].front(), Point(col, row)) < pixelDist(roadLabels[roadNumber].back(), Point(col , row)))
            {
                //reverse roadLabels[roadNumber]
                reverse(roadLabels[roadNumber].begin(), roadLabels[roadNumber].end());
                roadLabels[roadNumber].push_back(Point(col, row));
            }
            else
            {
                roadLabels[roadNumber].push_back(Point(col, row));
            }
        }

        roadFloodFillColor(colorImage, marked, row + 1, col, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row - 1, col, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row, col + 1, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row, col - 1, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row - 1, col - 1, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row - 1, col + 1, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row + 1, col - 1, color, roadNumber);
        roadFloodFillColor(colorImage, marked, row + 1, col + 1, color, roadNumber);
    }
}

void findContourPixels(Mat image, Mat marked, int row, int col, vector<Point> & contourPoints)
{
    // Finds and stores all 8 connected pixels from a particular pixel at (row,col)

    if (row >= image.rows || col >= image.cols || row < 0 || col < 0)
        return;

    if (image.at<uchar>(row,col) == 0 || marked.at<uchar>(row, col) == 255)
        return;

    contourPoints.push_back(Point(col, row));
    marked.at<uchar>(row, col) = 255;

    findContourPixels(image, marked, row + 1, col, contourPoints);
    findContourPixels(image, marked, row - 1, col, contourPoints);
    findContourPixels(image, marked, row, col + 1, contourPoints);
    findContourPixels(image, marked, row, col - 1, contourPoints);
    findContourPixels(image, marked, row - 1, col - 1, contourPoints);
    findContourPixels(image, marked, row - 1, col + 1, contourPoints);
    findContourPixels(image, marked, row + 1, col - 1, contourPoints);
    findContourPixels(image, marked, row + 1, col + 1, contourPoints);
}

void floodFillFindConnectedPixels(Mat image, int row, int col, vector<Point> * visited, Mat marked, int maxSize)
{
    // Method to identify and store all 4 connected pixels to the input pixel at position (row, col)

    if ((*visited).size() > maxSize + 10)
        return;

    if (row >= image.rows || col >= image.cols || row < 0 || col < 0)
        return;

    if (image.at<uchar>(row, col) == 255)
        return;

    if (find((*visited).begin(), (*visited).end(), Point(col, row)) != (*visited).end())
        return;

    marked.at<uchar>(row, col) = 255;
    (*visited).push_back(Point(col, row));

    floodFillFindConnectedPixels(image, row + 1, col, visited, marked, maxSize);
    floodFillFindConnectedPixels(image, row - 1, col, visited, marked, maxSize);
    floodFillFindConnectedPixels(image, row, col + 1, visited, marked, maxSize);
    floodFillFindConnectedPixels(image, row, col - 1, visited, marked, maxSize);
}

void findSpikeLength(Mat image, int row, int col, vector<Point> * linePoints, int spikeLengthThresh)
{
    // returns the constituent pixelsof a line which have a length < spikeLengthThreshold

    if (row >= image.rows || col >= image.cols || row < 0 || col < 0)
        return;

    if (image.at<uchar>(row, col) == 0)
        return;

    // If Point(col, row) exists in linePoints, return 0;
    if (find((*linePoints).begin(), (*linePoints).end(), Point(col, row)) != (*linePoints).end())
        return;

    if ((*linePoints).size() > 2 * spikeLengthThresh)
        return;

    if (neighbourCount(image, row, col) > 3)
    {
        return;
    }

    if (image.at<uchar>(row, col) == 255 && find((*linePoints).begin(), (*linePoints).end(), Point(col, row)) == (*linePoints).end())
    {
        (*linePoints).push_back(Point(col, row));
        findSpikeLength(image, row + 1, col, linePoints,spikeLengthThresh);
        findSpikeLength(image, row - 1, col, linePoints,spikeLengthThresh);
        findSpikeLength(image, row, col + 1, linePoints,spikeLengthThresh);
        findSpikeLength(image, row, col - 1, linePoints,spikeLengthThresh);
        findSpikeLength(image, row - 1, col - 1, linePoints,spikeLengthThresh);
        findSpikeLength(image, row - 1, col + 1, linePoints,spikeLengthThresh);
        findSpikeLength(image, row + 1, col - 1, linePoints,spikeLengthThresh);
        findSpikeLength(image, row + 1, col + 1, linePoints,spikeLengthThresh);
    }
}

void connectedPixels(Mat image, int row, int col, vector<Point> * visited, Mat marked)
{
    // Stores a list of pixels which are connected to the initial pixel at position (row,col)

    if ((*visited).size() > 260)
        return;

    if (row >= image.rows || col >= image.cols || row < 0 || col < 0)
        return;

    if (image.at<uchar>(row, col) == 0)
        return;

    if (find((*visited).begin(), (*visited).end(), Point(col, row)) != (*visited).end())
        return;

    marked.at<uchar>(row, col) = 255;
    (*visited).push_back(Point(col, row));

    connectedPixels(image, row + 1, col, visited, marked);
    connectedPixels(image, row - 1, col, visited, marked);
    connectedPixels(image, row, col + 1, visited, marked);
    connectedPixels(image, row, col - 1, visited, marked);
    connectedPixels(image, row - 1, col - 1, visited, marked);
    connectedPixels(image, row - 1, col + 1, visited, marked);
    connectedPixels(image, row + 1, col - 1, visited, marked);
    connectedPixels(image, row + 1, col + 1, visited, marked);
}
