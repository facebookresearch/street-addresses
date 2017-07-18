/*
 *  Copyright (c) 2017, Facebook, Inc.
 *  All rights reserved.
 *
 *  This file contains the main code for calling road segmentation methods.
 */

#include "road_segmentation.h"

int totalRoadCount = 90000;
vector<Point> roadLabels[90000];

int main(int argc, const char * argv[])
{
    string projectPath = argv[2];
    string filePath = argv[1];

    Mat image = imread(filePath, 0);

    //Resize the image to 0.5, however, stop processing if image.size() > 2^31 (Integer Overflow)
    if (image.size <= 0 || image.rows * image.cols > pow(2,30) || image.rows * image.cols <= 0)
    {
        cout << "Image size is either too big, or the image was not found. Terminating..." << endl;
        return 0;
    }
    resize(image, image, Size(), 0.5, 0.5, CV_INTER_AREA);
    threshold(image, image, 30, 255, CV_THRESH_BINARY);
    //imwrite(projectPath + "/1.ThreshTileImage.png", image);

    //Fill empty spaces in the binary image.
    cout << "Preprocessing..." << endl;
    image = fillGapsInBinaryImage(image, 60);
    //imwrite(projectPath + "/2.FillTileImage.png", image);

    //Convert to skeleton image
    cout << "Thinning Image";
    Mat zhangSuen = zhangSuenThinning(image);
    Mat guoHall = guoHallThinning(zhangSuen);
    Mat thinImage; guoHall.copyTo(thinImage);
    cout << endl; cout << "Thinning complete." << endl;
    //imwrite(projectPath + "/3.ThinTileImage.png", thinImage);

    cout << "Beginning road segmentation" << endl;
    //Assign road labels to individual road segments
    Mat labelImage = constructInitialRoadLabels(thinImage);

    //Bin Junction pixels along with the right road
    labelImage = binJunctionPixels(thinImage, labelImage);

    //Now fill the roadLabel array in ordered fashion
    labelImage = constructOrderedRoadLabels(labelImage);

    //Find corners in the image for connecting road segments
    vector<Point> cornerVector = findImageCorners(thinImage);

    //Connect roads across junctions and crossways
    labelImage = findContinuousRoads(labelImage, cornerVector);
    //imwrite(projectPath + "/4.RoadColorLabels.png", labelImage);

    writeJSON(labelImage, roadLabels, projectPath + "/roads.json");

    cout << "Finished" << endl;

    return 0;
}
