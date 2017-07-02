#include <iostream>
#include <dirent.h>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "LPRegistrar.h"

using namespace std;
using namespace cv ;
using namespace Registrar;

//////////////////////////////////////////////
enum input_t{VIDEO_IN, IMAGE_IN, CAMERA_IN};

string templ_dir_name = "/home/ahmad/Programs/Programming/CarLicensePlateDetection/templs/";
//////////////////////////////////////////////
Mat extractEdges(Mat frame);

bool isRectangle(Vec4i i, Vec4i i1);

Rect getRect(Vec4i i, Vec4i i1);

double lineLength(Vec4i vec);

void processFrame(const Mat &frame);

bool start_video_input(const String &fullFileName);

bool start_camera_input();

void start_input(String full_file_name, input_t input_type);

bool start_image_input(String full_file_name);

Mat get_input(input_t input_type);

Mat get_video_input();

Mat get_image_input();

void image_preprocessing(Mat &frame);

void image_sharpening(const Mat &frame);

void process_input_stream(const input_t &input_type, const String &fullFileName);

struct compareLines
{
    inline bool operator() (const Vec4i & l1, const Vec4i& l2)
    {
        return (l1[1] < l2[1]) || ((l1[1] == l2[1]) && (l1[0] < l2[0]));
    }
};

struct index_template
{
    Mat templ_mat;
    string templ_label;
};

vector<struct index_template> templs;

//////////////////////////////////////////////
bool has_more_input = false;
VideoCapture video;
string input_image_address;

int main(int argc, char** argv) {

    DIR * templ_dir = opendir(templ_dir_name.c_str());
    if(!templ_dir){
        cout << "Can not find templates directory :" << templ_dir_name << endl;
        exit(-1);
    }
    struct dirent * templ_dir_ent;
    while ((templ_dir_ent = readdir(templ_dir)) != NULL) {
        if(strcmp(templ_dir_ent->d_name,".") == 0 || strcmp(templ_dir_ent->d_name, "..") == 0)
            continue;
        String fullFileName = templ_dir_name + templ_dir_ent->d_name;

        Mat templ_mat = imread(fullFileName);
        string templ_label(templ_dir_ent->d_name);
        templ_label = templ_label.substr(0, templ_label.size() - 4);

        struct index_template new_index_template;
        new_index_template.templ_label = templ_label;
        new_index_template.templ_mat = templ_mat;

        templs.push_back(new_index_template);
    }
    closedir(templ_dir);

    string dataDirName = "/home/ahmad/Programs/Programming/CarLicensePlateDetection/Dataset" ;

    input_t input_type = VIDEO_IN;

    if(argc > 1){
        int input_index = 0;
        if(strcmp(argv[1], "--image") == 0) {
            input_type = IMAGE_IN;
            input_index ++;
        }
        if(strcmp(argv[1],"-c")==0 || argc == input_index+3){
            if(strcmp(argv[input_index+1] , "-d") == 0) {
                dataDirName = argv[input_index + 2];
                input_type = input_type == IMAGE_IN ? input_type : VIDEO_IN;
            }
            else if(strcmp(argv[input_index+1] , "-c") == 0) {
                input_type = CAMERA_IN;
            }
        } else {
            cout << "Wrong number of inputs" << endl;
            return -1;
        }
    }

    DIR * dir;
    struct dirent * ent;
    if (input_type != CAMERA_IN && (dir = opendir(dataDirName.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            cout << "***********************************" << endl;
            cout << "Processing file: " << ent->d_name << endl;
            String fullFileName = dataDirName + "/" + ent->d_name;

            process_input_stream(input_type, fullFileName);


        }
        closedir(dir);
    }else if(input_type == CAMERA_IN){
        process_input_stream(input_type, "");

    } else {
        /* could not open directory */
        perror ("could not open directory");
        return EXIT_FAILURE;
    }





    cout << "process has been finished." << endl ;
    return 0;
}

void process_input_stream(const input_t &input_type, const String &fullFileName) {
    start_input(fullFileName, input_type);

    while (has_more_input) {
                cout << "has more input" << endl;
                Mat frame = get_input(input_type);
                if (!frame.empty())
                    processFrame(frame);
            }
}

Mat get_input(input_t input_type) {
    if(input_type == VIDEO_IN || input_type == CAMERA_IN)
        return get_video_input();
    else
        return get_image_input();
}

Mat get_image_input() {
    has_more_input = false;
    return imread(input_image_address);
}

Mat get_video_input() {
    Mat frame;
    video >> frame ;
    has_more_input = !frame.empty();
    return frame;
}

void start_input(String full_file_name, input_t input_type) {
    if(input_type == VIDEO_IN)
        has_more_input = start_video_input(full_file_name);
    else if(input_type == CAMERA_IN)
        has_more_input = start_camera_input();
    else
        has_more_input = start_image_input(full_file_name);
}

bool start_image_input(String full_file_name) {
    input_image_address = full_file_name;
    return true;
}

bool start_video_input(const String &fullFileName) {
    cout << "trying to open video file '" << fullFileName << "' ..." << endl ;
    video = VideoCapture(fullFileName) ;
    return video.isOpened();
}

bool start_camera_input() {
    cout << "trying to open camera ..." << endl ;
    video = VideoCapture("rtsp://192.168.1.10:554/user=admin&password=&channel=1&stream=1.sdp?") ;
    return video.isOpened();
}

void processFrame(const Mat &input_frame) {
    namedWindow("edge frame", CV_WINDOW_KEEPRATIO);
    namedWindow("gray frame", CV_WINDOW_KEEPRATIO);

    Mat frame = input_frame;
//    imshow("input_frame",input_frame);

    image_preprocessing(frame);


    cout << "pyramiding input frame down..." << endl ;
    while(frame.cols > 500) {
        pyrDown(frame, frame) ;
    }
    cout << "reduced size: " << frame.cols << ", " << frame.rows << endl ;




//        cout << "extracting Canny edges from frame index " << frameIndex << endl;
    Mat edgeFrame = extractEdges(frame) ;

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
//
//        imshow("edge frame", houghFrame);

//    vector<vector<Point> > contours;
//    Mat bimage = frame >= sliderPos;
//    Mat bimage = grayFrame;
//    findContours(bimage, contours, RETR_LIST, CHAIN_APPROX_NONE);
//    Mat cimage = grayFrame;
//    for(size_t i = 0; i < contours.size(); i++)
//    {
//        size_t count = contours[i].size();
//        if( count < 6 )
//            continue;
//        cimage = Mat(contours[i]);
//        Mat apx = cimage.clone();
//        approxPolyDP(Mat(contours[i]), apx, 0.5,true);
//
//        InputArray * inputArray = new InputArray(contours[i]);
//        CvRect box = cvBoundingRect(inputArray);
//        if( MAX(box.width, box.height) > MIN(box.width, box.height)*30 )
//            continue;
//        drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);
//        ellipse(cimage, box, Scalar(0,0,255), 1, LINE_AA);
//        ellipse(cimage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(0,255,255), 1, LINE_AA);
//        line(cimage, Point(box.x, box.y), Point(box.x+box.width, box.y), Scalar(0,255,0), 1, LINE_AA);
//        line(cimage, Point(box.x+box.width, box.y), Point(box.x+box.width, box.y+box.height), Scalar(0,255,0), 1, LINE_AA);
//        line(cimage, Point(box.x, box.y), Point(box.x, box.y+box.height), Scalar(0,255,0), 1, LINE_AA);
//        line(cimage, Point(box.x, box.y + box.height), Point(box.x+box.width, box.y+box.height), Scalar(0,255,0), 1, LINE_AA);
//        cvtColor(apx, apx, CV_BGR2GRAY);
//        imshow("result", apx);
//        waitKey();
//    }

    int hist_size = 256;
    int hist_w = edgeFrame.cols; int hist_h = 100;
    int bin_w = cvRound( (double) hist_w/hist_size );

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

//    Mat hist;
//    float range[] = { 0, 256 } ;
//    const float* histRange = { range };
//    calcHist( &edgeFrame, 1, 0, Mat(), hist, 2, &hist_size, &histRange);
//
//
//    /// Normalize the result to [ 0, histImage.rows ]
//    normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
//    for( int i = 1; i < hist_size; i++ ) {
//        line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
//             Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
//             Scalar(255, 0, 0), 2, 8, 0);
//    }
//

    vector<int> hist_data;
    int max_sum = 0;

    for(int col = 0 ; col < edgeFrame.cols ; col ++){
        int sum = 0 ;
        for(int row = 0 ; row < edgeFrame.rows ; row ++){
            sum += edgeFrame.at<uchar>(row, col);
        }
        if(sum > max_sum)
            max_sum = sum ;
        hist_data.push_back(sum);
    }

    int hist_h_ratio = 100;
    for(int col = 1 ; col < edgeFrame.cols ; col ++){
        line(histImage, Point( (col - 1), hist_h - ((float)hist_data.at(col-1)/hist_h_ratio)),
             Point( col, hist_h - ((float)hist_data.at(col)/hist_h_ratio)),
             Scalar(255, 0, 0), 2, 8, 0);
//        line(histImage, Point(bin_w * (col - 1), hist_h - ((float)hist_data.at(col)/max_sum)),
//             Point(bin_w * col, hist_h - ((float)hist_data.at(col)/max_sum)),
//             Scalar(255, 0, 0), 2, 8, 0);
    }

    vector<Mat> splitted_characters;
    int start_index = 0, end_index = -1 ;
    bool select = false ;
    int search_bucket = 3;
    for(int i = search_bucket ; i < hist_data.size() ; i++){

        int sum = 0;
        for(int j = 0 ; j < search_bucket ; j++)
            sum += hist_data.at(i-j);
//        select = ((float)hist_data.at(i) / max_sum) > 40;

        select = ((float)sum)/search_bucket > 0;// && hist_data.at(i) < max_sum - 1000 ;

        if(select && start_index == 0) {
            start_index = i;
            end_index = 0;
        }
        else if((!select && end_index == 0)||(i==hist_data.size()-1)) {
            end_index = i;
            float with_ratio = (float)input_frame.cols / edgeFrame.cols;
            Rect cut_region(start_index*with_ratio,0,with_ratio*(end_index - start_index), input_frame.rows) ;
            Mat new_char = input_frame(cut_region);
            splitted_characters.push_back(new_char) ;
//            imshow("char", new_char);
//            moveWindow("char" , 100,100);
//            waitKey();
            start_index = 0;
            end_index = -1 ;

        }

    }

//    for (int i = 0; i < splitted_characters.size(); i++) {
//        Mat templ = (Mat &&) splitted_characters.at(i);
//        string templ_file_name = templ_dir_name + to_string(i) + ".png";
//        imwrite(templ_file_name , templ);
//    }

//
//        Mat templ = (Mat &&) splitted_characters.at(8);
//        string templ_file_name = templ_dir_name + to_string(1) + ".jpg";
//        imwrite(templ_file_name , templ);

    int match_method = 0;
    string car_license;
    for(int i = 0 ; i < splitted_characters.size(); i++){
        for(int j = 0 ; j < templs.size(); j++){
            Mat template_result;
            matchTemplate(splitted_characters.at(i), templs.at(j).templ_mat, template_result, match_method);
            normalize(template_result, template_result, 0, 1, NORM_MINMAX, -1, Mat() );

            cout << "splitted_index: " << i << ", template_label: " << templs.at(j).templ_label << ",result mean: " << mean(template_result) <<endl;

            if(mean(template_result)[0] < 0.1)
                car_license += templs.at(j).templ_label + "_";
        }
    }

    cout << "**************************************************" << endl ;
    cout << "**************************************************" << endl ;
    cout << "**** DETECTED_LICENSE: " << car_license << "************" << endl;
    cout << "**************************************************" << endl ;
    cout << "**************************************************" << endl ;


    imshow("edge frame", edgeFrame);
    imshow("gray frame", frame);
    imshow("hist", histImage);
    moveWindow("hist" , 100,100);

    waitKey() ;
}

void image_preprocessing(Mat &frame) {
    cvtColor(frame, frame, CV_BGR2HSV) ;

    Mat channels[3];
    split(frame, channels);
    channels[0] = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
    merge(channels, 3, frame);

    cvtColor(frame, frame, CV_HSV2BGR) ;
    cvtColor(frame, frame, CV_BGR2GRAY) ;

    GaussianBlur(frame, frame, Size(5, 5), 2 , 2);
//    GaussianBlur(frame, frame,Size(3,3), 1 , 1);
//    medianBlur(frame, frame,5);
//    medianBlur(frame, frame,3);
//    medianBlur(frame, frame,3);
    medianBlur(frame, frame,3);

    threshold(frame, frame, 100, 256, CV_THRESH_BINARY);

    equalizeHist(frame, frame);

//    image_sharpening(frame);
}

void image_sharpening(const Mat &frame) {
    Mat smooth_image;
    GaussianBlur(frame, smooth_image, Size(0, 0), 3);
    addWeighted(frame, 1.5, smooth_image, -0.5, 0, frame);
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

//    Canny(frame, edgeFrame, lowThreshold, ratio) ;

    Scharr(frame, edgeFrame, CV_8U, 1, 0, 100, 1);
//    Mat edgeFrame1;
//    Scharr(frame, edgeFrame1, CV_8U, 0, 1, 100, 1);
//    edgeFrame += edgeFrame1;

//    Mat edgeFrame1, edgeFrame2, edgeFrame3;
//    Sobel(frame, edgeFrame1, CV_8U, 1,0,5,100,1);
//    Sobel(frame, edgeFrame2, CV_8U, 0,1,5,100,1);
////    Sobel(frame, edgeFrame3, CV_8U, 1,1,5,100,1);
//    edgeFrame = edgeFrame1 + edgeFrame2 ;//+ edgeFrame3;

//    for(int i = 0 ; i < edgeFrame.rows ; i++) {
//        for (int j = 0; j < edgeFrame.cols; j++)
//            cout << edgeFrame.at<Vec2i>(i,j);
//    }

    Scalar threshold_scalar = mean(edgeFrame);
    double threshold_value = sum(threshold_scalar)[0];
    Mat thresholded_edge_frame = edgeFrame;
//    threshold(edgeFrame, thresholded_edge_frame, 256, 256, CV_THRESH_BINARY);
    threshold(edgeFrame, thresholded_edge_frame, threshold_value*2.5, 256, CV_THRESH_BINARY);

    return thresholded_edge_frame;
}
