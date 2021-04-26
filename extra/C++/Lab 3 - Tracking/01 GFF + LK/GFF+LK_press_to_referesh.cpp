#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

Point2f pt;
bool addRemovePt = false;

void onMouse( int event, int x, int y, int flags, void* param )
{
    if( event == EVENT_LBUTTONDOWN)
    {
        pt = Point2f((float)x,(float)y);
        addRemovePt = true;
    }
}

int main( int argc, char** argv )
{
    VideoCapture cap;
    TermCriteria termcrit(TermCriteria::MAX_ITER|TermCriteria::EPS ,20,0.03);
    Size winSize(10,10);

    int MAX_COUNT = 500;
    bool needToInit = false;
    bool nightMode = false;

    cap.open(0);
    //cap.open("Video.mp4");

    if( !cap.isOpened() )
    {
        cout << "Could not initialize capturing...\n";
        return 0;
    }

    // print a welcome message, and the OpenCV version
    cout << "Welcome to lkdemo, using OpenCV version %s\n" << CV_VERSION;

    cout << "\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tr - auto-initialize tracking\n"
            "\tc - delete all the points\n"
            "\tn - switch the \"night\" mode on/off\n"
            "To add/remove a feature point click it\n";

    namedWindow( "LK Demo", 1 );
    setMouseCallback( "LK Demo", onMouse, 0 );

    Mat gray, prevGray, image;
    vector<Point2f> points[2];

    for(;;)
    {
        Mat frame;
        cap >> frame;
        if( frame.empty() )
            break;

        frame.copyTo(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        if( nightMode )
            image = Scalar::all(0);

        if( needToInit )
        {
            // automatic initialization
            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, false, 0.04);
            //cornerSubPix(gray, points[1], winSize, Size(-1,-1), termcrit);
            addRemovePt = false;
        }
        else if( !points[0].empty() )
        {
            vector<uchar> status;
            vector<float> err;
            if(prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 3, termcrit, 0);
            
            size_t i, k;
            for( i = k = 0; i < points[1].size(); i++ )
            {
                if( addRemovePt )
                {
                    if( norm(pt - points[1][i]) <= 5 )
                    {
                        addRemovePt = false;
                        continue;
                    }
                }

                if( !status[i] )
                    continue;

                points[1][k++] = points[1][i];
                circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
            }
            points[1].resize(k);
        }

        if( addRemovePt && points[1].size() < (size_t)MAX_COUNT )
        {
            vector<Point2f> tmp;
            tmp.push_back(pt);
            cornerSubPix( gray, tmp, winSize, Size(-1,-1), termcrit);
            points[1].push_back(tmp[0]);
            addRemovePt = false;
        }

        needToInit = false;
        imshow("LK Demo", image);

        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch( c )
        {
        case 'r':
            needToInit = true;
            break;
        case 'c':
            points[1].clear();
            break;
        case 'n':
            nightMode = !nightMode;
            break;
        default:
            ;
        }

        std::swap(points[1], points[0]);
        swap(prevGray, gray);
    }

    return 0;
}
