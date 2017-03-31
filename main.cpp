#include <iostream>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv ;

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
    namedWindow(fullFileName, CV_WINDOW_KEEPRATIO) ;
    namedWindow(fullFileName + "(src)", CV_WINDOW_KEEPRATIO) ;
    int frameIndex = 0 ;
    while(! frame.empty()){
        imshow(fullFileName +  "(src)" , frame) ;

        Mat grayFrame ;
        cvtColor(frame, grayFrame, CV_BGR2GRAY) ;

        cout << "equalizing frame index " << frameIndex << endl ;
        equalizeHist(grayFrame, grayFrame) ;

        cout << "displaying frame index:" << frameIndex << endl ;
        imshow(fullFileName , grayFrame) ;
        waitKey(33) ;
        video >> frame ;
        frameIndex ++ ;
    }

    cout << "process has been finished." << endl ;
    return 0;
}
