#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv ;


Mat extractEdges(Mat frame);

bool isRectangle(Vec4i i, Vec4i i1);

Rect getRect(Vec4i i, Vec4i i1);

double lineLength(Vec4i vec);

struct compareLines
{
    inline bool operator() (const Vec4i & l1, const Vec4i& l2)
    {
        return (l1[1] < l2[1]) || ((l1[1] == l2[1]) && (l1[0] < l2[0]));
    }
};


int main() {
    cout << "starting program" << endl ;

    String dataDirName = "/home/ahmad/Programs/Programming/CarLicensePlateDetection/Dataset/" ;
    String fullFileName = dataDirName + "2.wmv" ;

    cout << "trying to open video file '" << fullFileName << "' ..." << endl ;
    VideoCapture video(fullFileName) ;

    if(!video.isOpened())
        throw "can not open file " + fullFileName ;

    cout << "reading video file frame by frame" << endl ;
    Mat frame ;
    video >> frame ;

    namedWindow("edge frame", CV_WINDOW_KEEPRATIO);
    namedWindow("gray frame", CV_WINDOW_KEEPRATIO);

    int frameIndex = 0 ;
    while(! frame.empty()){
        cout << "pyramiding down! index " << frameIndex << endl ;
        pyrDown(frame, frame) ;
        pyrDown(frame, frame) ;

        cout << "bluring frame index " << frameIndex << endl ;
        GaussianBlur(frame, frame,Size(5,5), 2 , 2);
        medianBlur(frame, frame,5);

        Mat grayFrame ;
        cvtColor(frame, grayFrame, CV_BGR2GRAY) ;


//        cout << "extracting Canny edges from frame index " << frameIndex << endl;
        Mat edgeFrame = extractEdges(grayFrame) ;

//        cout << "extracting hough lines" << endl ;
//        vector<Vec4i> houghLines ;
//        HoughLinesP(edgeFrame, houghLines, 0.1, CV_PI/180, 10,1,10) ;
//
//        cout << "detected hough lines: " << houghLines.size() << endl ;
//        Mat houghFrame(edgeFrame.rows, edgeFrame.cols, edgeFrame.type());
//        sort(houghLines.begin(), houghLines.end(), compareLines());
//        for (size_t i = 0; i < houghLines.size(); ++i)
//        {
//            Vec4i l = houghLines[i];
//            cout << l << endl ;
//            cout << houghFrame.rows << "," << houghFrame.cols << endl ;
//            if((fabs(l[1] - l[3]) < 5 || fabs(l[0] - l[2]) < 5) && lineLength(l) < 40 && lineLength(l) > 5)
//                line(houghFrame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 255, 255), 1, LINE_AA);
//            if(i > 1 && isRectangle(houghLines[i-1], houghLines[i])){
//                line(grayFrame, Point(houghLines[i-1][0], houghLines[i-1][1]), Point(houghLines[i-1][2], houghLines[i-1][3]), Scalar(255, 255, 255), 1, LINE_AA);
//                line(grayFrame, Point(houghLines[i][0], houghLines[i][1]), Point(houghLines[i][2], houghLines[i][3]), Scalar(255, 255, 255), 1, LINE_AA);
//            }
//        }

//        imshow("edge frame", houghFrame);
        imshow("edge frame", edgeFrame);
        imshow("gray frame", grayFrame);
        waitKey() ;


        video >> frame ;
        frameIndex ++ ;
    }

    cout << "process has been finished." << endl ;
    return 0;
}

Rect getRect(Vec4i l1, Vec4i l2) {
    double width = max(lineLength(l1),lineLength(l2)), height = min(lineLength(l2),lineLength(l1)) ;
    Rect rect(l1[0], l1[1], (int)width,(int) height) ;
    return rect;
}

bool isRectangle(Vec4i l1, Vec4i l2) {
    double lengthRatio = lineLength(l1)/lineLength(l2) ;
    return fabs(l1[1] - l2[1]) < 10 && fabs(l1[0] - l2[0]) < 10 &&
           (fabs(l1[1] - l1[3]) < 5 || fabs(l1[0] - l1[2]) < 5)
           &&
            (fabs(l2[1] - l2[3]) < 5 || fabs(l2[0] - l2[2]) < 5)
            &&
            ((lengthRatio < 6 && lengthRatio > 4)
            ||
            (lengthRatio > 1/6 && lengthRatio < 1/4));
}

double lineLength(Vec4i line) {
    return sqrt(pow((line[0] - line[2]),2) + pow((line[1] - line[3]),2));
}

Mat extractEdges(Mat frame) {
    Mat edgeFrame;
    int lowThreshold = 100 ;
    int ratio = 2;

    Canny(frame, edgeFrame, lowThreshold, ratio) ;

    return edgeFrame;
}
