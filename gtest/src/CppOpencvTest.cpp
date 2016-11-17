#ifndef __CYGWIN__
#include <limits>

#include "gtest/gtest.h"
#include "opencv/opencv2/opencv.hpp"
#include "opencv2/core/cvstd.hpp"

#include "CppCurl.h"
#include "CppFile.h"
#include "CppMath.h"

using namespace std;
using namespace cv;

int32_t GetDropPoint(const Mat &mat, const Point &currPoint, const Point &lastPoint, Point &nextPoint,
                     bool dontToLeft)
{
    // 水滴优先往黑色的像素点流动
    // TODO 限制水滴的横向范围

    // 获得点的状态，白的为true，表示有内容
    // 0 p 1
    // 2 3 4
    uint8_t pointStatus[5];

    // 下方没有点，返回-1
    if (currPoint.y >= mat.rows - 1)
    {
        return -1;
    }

    if (static_cast<int32_t>(currPoint.x) >= mat.cols)
    {
        return -1;
    }

    if (currPoint.x == 0)
    {
        // 如果左边没有点了，则左边为黑色，表示可滴入
        pointStatus[0] = true;
        pointStatus[2] = true;
    }
    else
    {
        // 左边有点
        if (currPoint.y >= 0)
        {
            pointStatus[0] = mat.at<uint8_t>(currPoint.y, currPoint.x - 1);
        }
        else
        {
            // 第一行上面那一行，都可以流动
            pointStatus[0] = false;
        }

        pointStatus[2] = mat.at<uint8_t>(currPoint.y + 1, currPoint.x - 1);
    }

    if (static_cast<int32_t>(currPoint.x) == mat.cols - 1)
    {
        // 如果右边没有点了，则右边为false
        pointStatus[1] = true;
        pointStatus[4] = true;
    }
    else
    {
        // 右边有点
        if (currPoint.y >= 0)
        {
            pointStatus[1] = mat.at<uint8_t>(currPoint.y, currPoint.x + 1);
        }
        else
        {
            // 第一行上面那一行，都可以流动
            pointStatus[1] = false;
        }

        pointStatus[4] = mat.at<uint8_t>(currPoint.y + 1, currPoint.x + 1);
    }

    pointStatus[3] = mat.at<uint8_t>(currPoint.y + 1, currPoint.x);

    /* 判断滴入方向 */
    // 1、下方是黑的，往下滴
    if (!pointStatus[3])
    {
        nextPoint.y = currPoint.y + 1;
        nextPoint.x = currPoint.x;
        return 0;
    }

    // 2、周边全是白的，往下滴
    if (pointStatus[0] && pointStatus[1] && pointStatus[2] && pointStatus[3] && pointStatus[4])
    {
        nextPoint.y = currPoint.y + 1;
        nextPoint.x = currPoint.x;
        return 0;
    }

    // 3、下方是白的，下方周边是黑的，往黑的方向滴，优先右侧
    if (!pointStatus[4])
    {
        nextPoint.y = currPoint.y + 1;
        nextPoint.x = currPoint.x + 1;
        return 0;
    }

    if (!pointStatus[2])
    {
        nextPoint.y = currPoint.y + 1;
        nextPoint.x = currPoint.x - 1;
        return 0;
    }

    // 4、下方全是白的，只能往左右滴，优先右侧
    // 如果是从右边过来的，并且左侧可移动（黑色的），则不走这条分支
    if (!pointStatus[1]
        && !(lastPoint.x == currPoint.x + 1 && lastPoint.y == currPoint.y &&!pointStatus[0]))
    {
        nextPoint.y = currPoint.y;
        nextPoint.x = currPoint.x + 1;
        return 0;
    }

    // 如果必须往下，则往下，避免左右循环
    if (dontToLeft)
    {
        nextPoint.y = currPoint.y + 1;
        nextPoint.x = currPoint.x;
        return 0;
    }

    // 否则往左流动
    nextPoint.y = currPoint.y;
    nextPoint.x = currPoint.x - 1;
    return 0;
}

void GetDropMat(const Mat &matSrc, Mat &matOut, vector<vector<Point>> &dropPoints)
{
    dropPoints.clear();
    matSrc.copyTo(matOut);
    int32_t ret = 0;
    bool dontToLeft;

    // 找到左右侧开始的点
    int32_t startCol = -1;
    int32_t endCol = -1;
    for (uint32_t c = 0; static_cast<int32_t>(c) < matSrc.cols && startCol == -1; ++c)
    {
        for (uint32_t r = 0; static_cast<int32_t>(r) < matSrc.rows; ++r)
        {
            if (matSrc.at<uint8_t>(r, c) != 0)
            {
                startCol = c;
                break;
            }
        }
    }

    for (int32_t c = matSrc.cols - 1; c >= 0 && endCol == -1; --c)
    {
        for (uint32_t r = 0; static_cast<int32_t>(r) < matSrc.rows; ++r)
        {
            if (matSrc.at<uint8_t>(r, c) != 0)
            {
                endCol = c;
                break;
            }
        }
    }

    static uint32_t CODE_WIDTH = (endCol - startCol) / 5;

    for (int32_t c = CODE_WIDTH / 3; c < static_cast<int32_t>(matOut.cols - CODE_WIDTH / 3); ++c)
    {
        // 从-1行开始滴入，避免直接穿透图
        Point currPoint(c, -1);;
        Point lastPoint = currPoint;
        Point nextPoint;
        Point recordPoint;
        bool haveFirstDrop = false;
        bool haveLine = false;
        vector<Point> *pCurrDorpPoints = NULL;

        while (true)
        {
            // 如果是从左边流过来的，并且已经2次，则不再往左流动
            dontToLeft = (recordPoint.y == currPoint.y && recordPoint.x == currPoint.x);

            // 首次白色穿透点，标记
            if (currPoint.y >= 0)
            {
                if (matSrc.at<uint8_t>(currPoint) == 255 && !haveFirstDrop)
                {
                    haveFirstDrop = true;
                    matOut.at<uint8_t>(currPoint) = 200;

                    // 查找有没有这个点流入的线
                    for (auto points_it = dropPoints.begin(); points_it != dropPoints.end(); ++points_it)
                    {
                        if (*points_it->begin() == currPoint)
                        {
                            haveLine = true;
                            break;
                        }
                    }

                    // 没找到，添加一条
                    if (!haveLine)
                    {
                        dropPoints.push_back(vector<Point>());
                        pCurrDorpPoints = &(*dropPoints.rbegin());
                        pCurrDorpPoints->push_back(currPoint);
                    }
                }
                else if (matOut.at<uint8_t>(currPoint) != 200)
                {
                    // 标记点
                    matOut.at<uint8_t>(currPoint) = 128;
                    if (pCurrDorpPoints != NULL)
                    {
                        pCurrDorpPoints->push_back(currPoint);
                    }
                }
                else
                {

                }
            }

            ret = GetDropPoint(matSrc, currPoint, lastPoint, nextPoint, dontToLeft);
            if (ret != 0)
            {
                break;
            }

            // 如果从左边流过来，并且本次是流向左边，则记录一次
            if (lastPoint.y == currPoint.y && lastPoint.x == currPoint.x - 1
                && nextPoint.y == currPoint.y && nextPoint.x == currPoint.x - 1)
            {
                recordPoint.y = currPoint.y;
                recordPoint.x = currPoint.x;
            }

            lastPoint.y = currPoint.y;
            lastPoint.x = currPoint.x;
            currPoint.y = nextPoint.y;
            currPoint.x = nextPoint.x;
        }
    }

    // 绘制辅助线
    //     for (uint32_t c = startCol; static_cast<int32_t>(c) <= endCol; c += CODE_WIDTH)
    //     {
    //         for (uint32_t r = 0; static_cast<int32_t>(r) < matDrop.rows; r += 2)
    //         {
    //             matDrop.at<uint8_t>(r, c) = 200;
    //         }
    //     }
    // 
    //     imwrite("./data/codeimg_drop_with_line.jpg", matDrop);
}

/** 获得每个起始点对应的
 *
 * @param   const Point & firstPoint                    当前起始点
 * @param   const vector<vector<Point>> & opLines       反方向的线条集合
 * @param   vector<Point> * & pLine                     当前起始点对应的匹配线
 * @param   Point * & startMatchPoint                   当前起始点对应匹配线最近的点
 * @retval  int32_t
 * @author  moontan
 */
int32_t GetMatchLine(const Point &firstPoint, const vector<vector<Point>> &opLines,
                     const vector<Point> *&pMatchLine, Point &startMatchPoint)
{
    static uint32_t MAX_X_DISTANCE = 30;
    pMatchLine = NULL;

    uint32_t minStart2StartX = (numeric_limits<uint32_t>::max)();
    for (auto opLineIt = opLines.begin(); opLineIt != opLines.end(); ++opLineIt)
    {
        // 计算横向距离
        uint32_t distance = abs(firstPoint.x - opLineIt->begin()->x);
        if (minStart2StartX > distance && distance <= MAX_X_DISTANCE)
        {
            minStart2StartX = distance;
            pMatchLine = &(*opLineIt);
        }
    }

    // 如果找到，查找当前起点对应的线条中最小的那条
    double minStart2Point = (numeric_limits<double>::max)();
    if (pMatchLine != NULL)
    {
        for (auto point_it = pMatchLine->begin(); point_it != pMatchLine->end(); ++point_it)
        {
            double distance = cv::norm(firstPoint - *point_it);
            //                 printf("(%d,%d) - (%d,%d) = %lf\n", upIt->begin()->x, upIt->begin()->y,
            //                        point_it->x, point_it->y, distance);
            if (distance < minStart2Point)
            {
                startMatchPoint = *point_it;
                minStart2Point = distance;
            }
        }
    }

    if (pMatchLine != NULL)
    {
        return 0;
    }

    return -1;
}

struct DropLineInfo
{
    const vector<Point> *pDropLine;
    const vector<Point> *pMatchLine;
    Point MatchPoint;
    bool useThisLine;

    DropLineInfo() :pDropLine(NULL), pMatchLine(NULL), useThisLine(false)
    {
    }
};

void CheckMatch(vector<DropLineInfo> &upDropLineInfo, vector<DropLineInfo> &downDropLineInfo)
{
    for (auto upDropInfoIt = upDropLineInfo.begin(); upDropInfoIt != upDropLineInfo.end(); ++upDropInfoIt)
    {
        if (upDropInfoIt->pMatchLine != NULL)
        {
            for (auto downDropInfoIt = downDropLineInfo.begin(); downDropInfoIt != downDropLineInfo.end(); ++downDropInfoIt)
            {
                // 上匹配到了下
                if (*upDropInfoIt->pMatchLine->begin() == *downDropInfoIt->pDropLine->begin())
                {
                    // 但是下没匹配到上
                    // TODO 这里会core
                    if (downDropInfoIt->pMatchLine == NULL || *downDropInfoIt->pMatchLine->begin() != *upDropInfoIt->pDropLine->begin())
                    {
                        // 清除信息
                        upDropInfoIt->pMatchLine = NULL;
                    }

                    break;
                }
            }
        }
    }
}

void DrawLine(Mat &mat, vector<DropLineInfo> &upDropLineInfo, vector<DropLineInfo> &downDropLineInfo)
{
    for (auto upDropInfoIt = upDropLineInfo.begin(); upDropInfoIt != upDropLineInfo.end(); ++upDropInfoIt)
    {
        // 当没有匹配线或者有匹配线，但是选择了本条线，绘制
        if (upDropInfoIt->pMatchLine == NULL || upDropInfoIt->useThisLine)
        {
            DropLineInfo *pDownDropInfo = NULL;
            if (upDropInfoIt->pMatchLine != NULL)
            {
                for (auto downDropInfoIt = downDropLineInfo.begin(); downDropInfoIt != downDropLineInfo.end(); ++downDropInfoIt)
                {
                    if (*upDropInfoIt->pMatchLine->begin() == *downDropInfoIt->pDropLine->begin())
                    {
                        pDownDropInfo = &(*downDropInfoIt);
                        break;
                    }
                }
            }

            for (auto pointIt = upDropInfoIt->pDropLine->begin(); pointIt != upDropInfoIt->pDropLine->end(); ++pointIt)
            {
                // 达到对方顶点匹配点时，绘制连接线，跳出
                if (pDownDropInfo != NULL &&*pointIt == pDownDropInfo->MatchPoint)
                {
                    line(mat, *pointIt, *pDownDropInfo->pDropLine->begin(), Scalar(128));
                    break;
                }

                mat.at<uint8_t>(*pointIt) = 128;
            }
        }
    }
}

// TEST(CppOpencv, Binary)
// {
//     static const uint32_t IMAGE_COUNT = 500;
//     static const string FILE_NAME = "./data/codeimg.jpg";
// 
//     // 下载图片
// //     try
// //     {
// //         for (uint32_t i = 0; i < IMAGE_COUNT; ++i)
// //         {
// //             string imgData = CppCurl::Get("http://www.docin.com/servlet/getimg?vfrom=loginFlase");
// //             CppFile::WriteToFile(CppString::GetArgs("./data/docincodes/%u.jpg", i), imgData);
// //             printf("%u\n", i);
// //             usleep(300000);
// //         }
// //         return;
// //     }
// //     catch (...)
// //     {
// //         return;
// //     }
// 
//     // 获得图片
//     uint32_t imgId = CppMath::Random(0, IMAGE_COUNT);
//     CppFile::CopyFile(CppString::GetArgs("./data/docincodes/%u.jpg", imgId), FILE_NAME);
// 
//     // 从文件中加载原图
//     Ptr<IplImage> iplimg(cvLoadImage("./data/codeimg.jpg"));
//     EXPECT_TRUE(iplimg != NULL);
//     Mat matSrc = cvarrToMat(iplimg);
// 
//     // 转化为灰度图
//     Mat matGray;
//     cvtColor(matSrc, matGray, CV_BGR2GRAY);
//     //imwrite("./data/codeimg_gray.jpg", matGray);
// 
//     /* 二值化图像 */
//     // 寻找阈值
//     // 寻找最点的大小，在此基础上，变白40%即为阈值
//     uint8_t darkestValue = 255;
//     for (auto it = matGray.begin<uint8_t>(); it != matGray.end<uint8_t>(); ++it)
//     {
//         if (darkestValue > *it)
//         {
//             darkestValue = *it;
//         }
//     }
// 
//     Mat matBin;
//     threshold(matGray, matBin, darkestValue + (255 - darkestValue) * 40 / 100, 255, THRESH_BINARY_INV);
//     imwrite("./data/codeimg_bin.jpg", matBin);
// 
//     // 创建投影图
//     Mat matX = Mat::zeros(matBin.size(), CV_8UC1);
//     for (int32_t c = 0; c < matBin.cols; ++c)
//     {
//         int32_t outRow = matBin.rows;
//         for (int32_t r = 0; r < matBin.rows; ++r)
//         {
//             if (matBin.at<uint8_t>(r, c) == 255)
//             {
//                 matX.at<uint8_t>(--outRow, c) = 255;
//             }
//         }
//     }
//     //imwrite("./data/codeimg_x.jpg", matX);
// 
//     // 腐蚀图像
//     Mat matErode;
//     erode(matBin, matErode, Mat(6, 6, CV_8U, cv::Scalar(1)));
//     //imwrite("./data/codeimg_erode.jpg", matErode);
// 
//     // 大腐蚀小膨胀
//     Mat matErode2;
//     erode(matBin, matErode2, Mat(6, 6, CV_8U, cv::Scalar(1)));
//     dilate(matErode2, matErode2, Mat(4, 4, CV_8U, cv::Scalar(1)));
//     //imwrite("./data/codeimg_erode2.jpg", matErode2);
// 
//     // bin图减去大腐蚀小膨胀的结果
//     Mat matMinus = matBin - matErode2;
//     //imwrite("./data/codeimg_minus.jpg", matMinus);
// 
//     // 开运算
//     Mat matMorphOpen;
//     morphologyEx(matBin, matMorphOpen, cv::MORPH_OPEN, Mat());
//     //imwrite("./data/codeimg_morph_open.jpg", matMorphOpen);
// 
//     // 闭运算
//     Mat matMorphClose;
//     Mat matMorphClose2;
//     morphologyEx(matBin, matMorphClose, cv::MORPH_CLOSE, Mat());
//     morphologyEx(matMorphClose, matMorphClose2, cv::MORPH_CLOSE, Mat());
//     //imwrite("./data/codeimg_morph_close.jpg", matMorphClose2);
// 
//     // 寻找滴水点
//     Mat matDrop;
//     vector<vector<Point>> upDropLines;
//     GetDropMat(matBin, matDrop, upDropLines);
//     imwrite("./data/codeimg_drop.jpg", matDrop);
// 
//     // 水滴倒滴
//     Mat matFlip;
//     flip(matBin, matFlip, 0);
//     //imwrite("./data/codeimg_flip.jpg", matFlip);
// 
//     Mat matFlipDrop;
//     vector<vector<Point>> downDropLines;
//     GetDropMat(matFlip, matFlipDrop, downDropLines);
//     flip(matFlipDrop, matFlipDrop, 0);
//     imwrite("./data/codeimg_flip_drop.jpg", matFlipDrop);
// 
//     // 倒转y轴并且标记到mark图
//     Mat matMark;
//     matBin.copyTo(matMark);
//     for (auto it = upDropLines.begin(); it != upDropLines.end(); ++it)
//     {
//         matMark.at<uint8_t>(*it->begin()) = 200;
//     }
// 
//     for (auto it = downDropLines.begin(); it != downDropLines.end(); ++it)
//     {
//         for (auto point_it = it->begin(); point_it != it->end(); ++point_it)
//         {
//             point_it->y = matBin.rows - 1 - point_it->y;
//         }
//         matMark.at<uint8_t>(*it->begin()) = 200;
//     }
// 
//     // 添加线条
// //     for (auto points_it = upDropLines.begin(); points_it != upDropLines.end(); ++points_it)
// //     {
// //         for (auto point_it = points_it->begin(); point_it != points_it->end(); ++point_it)
// //         {
// //             matMark.at<uint8_t>(*point_it) = 100;
// //         }
// //     }
// // 
// //     for (auto points_it = downDropLines.begin(); points_it != downDropLines.end(); ++points_it)
// //     {
// //         for (auto point_it = points_it->begin(); point_it != points_it->end(); ++point_it)
// //         {
// //             matMark.at<uint8_t>(*point_it) = 150;
// //         }
// //     }
// 
//     // 连接上下滴穿点
//     // 针对每个上滴穿点，找一个与它最近的下滴穿点，并且距离不能超过4，找到后与其相连
// //     for (auto upLineIt = upDropLines.begin(); upLineIt != upDropLines.end(); ++upLineIt)
// //     {
// //         double minDistance = (numeric_limits<double>::max)();
// //         Point targetPoint;
// //         for (auto downIt = downDropLines.begin(); downIt != downDropLines.end(); ++downIt)
// //         {
// //             // 计算角度
// //             //double angle = fabs(atan(static_cast<double>((upIt->begin()->x - downIt->begin()->x)) / (upIt->begin()->y - downIt->begin()->y))) * 180 / 3.14;
// //             //double distance = cv::norm(*upIt->begin() - *downIt->begin());
// //             double distance = fabs(upLineIt->begin()->x - downIt->begin()->x);
// //             //printf("(%d,%d) - (%d,%d) = %lf,angle = %.0lf\n", upIt->begin()->x, upIt->begin()->y, downIt->begin()->x, downIt->begin()->y, distance, angle);
// //             if (minDistance > distance && distance < 6)
// //             {
// //                 minDistance = distance;
// //                 targetPoint = *downIt->begin();
// //             }
// //         }
// // 
// //         // 如果找到，连线到mark图
// //         if (minDistance != (numeric_limits<double>::max)())
// //         {
// //             line(matMark, *upLineIt->begin(), targetPoint, Scalar(200));
// //         }
// //     }
// 
// 
//     // 方法1：对比匹配的从上到下流出的点和从下到上流入的点，近的那一条，将尾端与另一条首端相连
//     // 
//     // 方法2：对于每个起点，找到对应的那一条流线中，距离最近的一个点，连接起来
//     vector<DropLineInfo> upDropLineInfo;
//     vector<DropLineInfo> downDropLineInfo;
// 
//     // 拿到所有上流水线对应的下流水线信息
//     for (auto upLineIt = upDropLines.begin(); upLineIt != upDropLines.end(); ++upLineIt)
//     {
//         DropLineInfo dropLineInfo;
//         dropLineInfo.pDropLine = &(*upLineIt);
//         (void)GetMatchLine(*upLineIt->begin(), downDropLines, dropLineInfo.pMatchLine, dropLineInfo.MatchPoint);
//         upDropLineInfo.push_back(std::move(dropLineInfo));
//     }
// 
//     // 拿到所有下流水线对应的上流水线信息
//     for (auto downLineIt = downDropLines.begin(); downLineIt != downDropLines.end(); ++downLineIt)
//     {
//         DropLineInfo dropLineInfo;
//         dropLineInfo.pDropLine = &(*downLineIt);
//         (void)GetMatchLine(*downLineIt->begin(), upDropLines, dropLineInfo.pMatchLine, dropLineInfo.MatchPoint);
//         downDropLineInfo.push_back(std::move(dropLineInfo));
//     }
// 
//     // 解决匹配流水线冲突，比如一条线对应多条线的情况，这里可能需要进一步规划，因为取消后，可能还有其他匹配值，可通过存储多个匹配线解决
//     CheckMatch(upDropLineInfo, downDropLineInfo);
//     CheckMatch(downDropLineInfo, upDropLineInfo);
// 
//     // 选择匹配的线
//     for (auto upDropInfoIt = upDropLineInfo.begin(); upDropInfoIt != upDropLineInfo.end(); ++upDropInfoIt)
//     {
//         if (upDropInfoIt->pMatchLine != NULL)
//         {
//             // 选择末端距离对方其实点更近一点的那条线
//             double upStartDistance = norm(*upDropInfoIt->pDropLine->begin() - upDropInfoIt->MatchPoint);
//             double downStartDistance = 0;
//             auto downDropInfoIt = downDropLineInfo.begin();
//             for (; downDropInfoIt != downDropLineInfo.end(); ++downDropInfoIt)
//             {
//                 if (*upDropInfoIt->pMatchLine->begin() == *downDropInfoIt->pDropLine->begin())
//                 {
//                     downStartDistance = norm(*downDropInfoIt->pDropLine->begin() - downDropInfoIt->MatchPoint);
//                     break;
//                 }
//             }
// 
//             if (downDropInfoIt == downDropLineInfo.end())
//             {
//                 printf("error!\n");
//                 return;
//             }
// 
//             // 选择流水线
//             upDropInfoIt->useThisLine = upStartDistance > downStartDistance;
//             downDropInfoIt->useThisLine = !upDropInfoIt->useThisLine;
//         }
//     }
// 
//     // 连接上下点
//     for (auto upDropInfoIt = upDropLineInfo.begin(); upDropInfoIt != upDropLineInfo.end(); ++upDropInfoIt)
//     {
//         if (upDropInfoIt->pMatchLine != NULL)
//         {
//             line(matMark, *upDropInfoIt->pDropLine->begin(), *upDropInfoIt->pMatchLine->begin(), Scalar(200));
//         }
//     }
//     imwrite("./data/codeimg_mark.jpg", matMark);
// 
//     // 绘图
//     Mat matMethod2;
//     matBin.copyTo(matMethod2);
//     DrawLine(matMethod2, upDropLineInfo, downDropLineInfo);
//     DrawLine(matMethod2, downDropLineInfo, upDropLineInfo);
// 
//     //     for (auto point_it = pMatchPoints->begin(); point_it != pMatchPoints->end(); ++point_it)
//     //     {
//     //         // 找到匹配的那个点了，就跳出
//     //         if (*point_it == minOpPoint)
//     //         {
//     //             break;
//     //         }
//     //         matMethod2.at<uint8_t>(*point_it) = 128;
//     //     }
//     // 
//     //     if (minDistance2 != (numeric_limits<double>::max)())
//     //     {
//     //         // 连线
//     //         line(matMethod2, minStartPoint, minOpPoint, Scalar(128));
//     //     }
//     // 
//     //     // 反向再来一次
//     //     for (auto downIt = downDropLines.begin(); downIt != downDropLines.end(); ++downIt)
//     //     {
//     //         // 已经匹配过，跳过
//     //         auto find_it = find(matchedStartPoints.begin(), matchedStartPoints.end(), *downIt->begin());
//     //         if (find_it != matchedStartPoints.end())
//     //         {
//     //             continue;
//     //         }
//     // 
//     //         // 标记到图
//     //         for (auto point_it = downIt->begin(); point_it != downIt->end(); ++point_it)
//     //         {
//     //             matMethod2.at<uint8_t>(*point_it) = 128;
//     //         }
//     //     }
// 
//     imwrite("./data/codeimg_methond2.jpg", matMethod2);
// 
//     // 对图像做开操作，消除多余的孤立点
// //     Mat matMethod2Open;
// //     morphologyEx(matMethod2, matMethod2Open, cv::MORPH_OPEN, Mat(2, 2, CV_8U, cv::Scalar(1)));
// // //     erode(matMethod2, matMethod2Open, Mat(2, 2, CV_8U, cv::Scalar(1)));
// //     dilate(matMethod2Open, matMethod2Open, Mat(2, 2, CV_8U, cv::Scalar(1)));
//  //   imwrite("./data/codeimg_methond2_open.jpg", matMethod2Open);
// 
// 
//     // 方法3：重新流动，穿透点时优先考虑靠近连接线的点
// }

#endif
