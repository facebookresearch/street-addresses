/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 *  This file contains code for performing the ZhangSuen and GuoHall image thinning.
 */

#include "RoadSegmentation.h"

void thinningGuoHallIteration(Mat & im, int iter)
{
    // Method called while performing a specific
    // iteration of the Guo Hall thinning.

    Mat marker = Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows - 1; i++)
    {
        for (int j = 1; j < im.cols - 1; j++)
        {
            uchar p2 = im.at<uchar>(i - 1, j);
            uchar p3 = im.at<uchar>(i - 1, j + 1);
            uchar p4 = im.at<uchar>(i, j + 1);
            uchar p5 = im.at<uchar>(i + 1, j + 1);
            uchar p6 = im.at<uchar>(i + 1, j);
            uchar p7 = im.at<uchar>(i + 1, j - 1);
            uchar p8 = im.at<uchar>(i, j - 1);
            uchar p9 = im.at<uchar>(i - 1, j - 1);

            int C = ((!p2) & (p3 | p4)) +
            ((!p4) & (p5 | p6)) +
            ((!p6) & (p7 | p8)) +
            ((!p8) & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N = N1 < N2 ? N1 : N2;
            int m = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

            if (C == 1 && (N >= 2 && N <= 3) && m == 0)
                marker.at<uchar>(i, j) = 1;
        }
    }
    im &= ~marker;
}

Mat guoHallThinning(Mat im)
{
    // Method returns a thin skeletonized image from a given input image.

    //Convert to grayscale
    if (im.channels() > 1)
        cvtColor(im, im, CV_BGR2GRAY);

    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    int count = 0;
    do {
        thinningGuoHallIteration(im, 0);
        thinningGuoHallIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
        cout << "." ;
    } while (cv::countNonZero(diff) > 0);

    im *= 255;

    for (int i = 0; i < im.rows; i++)
    {
        for (int j = 0; j < im.cols; j++)
        {
            if (i == 0 || j == 0 || i == im.rows - 1 || j == im.cols - 1)
                im.at<uchar>(i, j) = 0;

            if (i == 0 && im.at<uchar>(i + 1, j) == 255) //Top Row
                im.at<uchar>(i, j) = 255;

            if (i == im.rows - 1 && im.at<uchar>(i - 1, j) == 255) // Last row
                im.at<uchar>(i, j) = 255;

            if (j == 0 && im.at<uchar>(i, j + 1) == 255) // Left col
                im.at<uchar>(i, j) = 255;

            if (j == im.cols - 1 && im.at<uchar>(i, j - 1) == 255) //Right col
                im.at<uchar>(i, j) = 255;
        }
    }
    return im;
}

void thinningZhangSuenIteration(Mat & im, int iter)
{
    // Method called while performing a specific
    // iteration of the Zhang Suen thinning.

    Mat marker = Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows - 1; i++)
    {
        for (int j = 1; j < im.cols - 1; j++)
        {
            uchar p2 = im.at<uchar>(i - 1, j);
            uchar p3 = im.at<uchar>(i - 1, j + 1);
            uchar p4 = im.at<uchar>(i, j + 1);
            uchar p5 = im.at<uchar>(i + 1, j + 1);
            uchar p6 = im.at<uchar>(i + 1, j);
            uchar p7 = im.at<uchar>(i + 1, j - 1);
            uchar p8 = im.at<uchar>(i, j - 1);
            uchar p9 = im.at<uchar>(i - 1, j - 1);

            int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
            (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
            (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
            (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i, j) = 1;
        }
    }
    im &= ~marker;
}

Mat zhangSuenThinning(Mat im)
{
    // Zhang Suen Thinning Method for perfoming image skeletonization.

    //Convert to grayscale
    if (im.channels() > 1)
        cvtColor(im, im, CV_BGR2GRAY);

    im /= 255;

    Mat prev = Mat::zeros(im.size(), CV_8UC1);
    Mat diff;

    int count = 0;
    do {
        thinningZhangSuenIteration(im, 0);
        thinningZhangSuenIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
        cout << ".";
    } while (cv::countNonZero(diff) > 0);

    im *= 255;

    for (int i = 0; i < im.rows; i++)
    {
        for (int j = 0; j < im.cols; j++)
        {
            if (i == 0 || j == 0 || i == im.rows - 1 || j == im.cols - 1)
            {
                im.at<uchar>(i, j) = 0;
            }

            if (i == 0 && im.at<uchar>(i+1,j) == 255) //Top Row
            {
                im.at<uchar>(i, j) = 255;
            }
            if (i == im.rows - 1 && im.at<uchar>(i-1, j) == 255) // Last row
            {
                im.at<uchar>(i, j) = 255;
            }
            if (j == 0 && im.at<uchar>(i, j + 1) == 255) // Left col
            {
                im.at<uchar>(i, j) = 255;
            }
            if (j == im.cols - 1 && im.at<uchar>(i, j - 1) == 255) //Right col
            {
                im.at<uchar>(i, j) = 255;
            }
        }
    }
    return im;
}
