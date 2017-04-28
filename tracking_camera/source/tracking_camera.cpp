#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

Mat image;

auto backprojMode = false;
auto selectObject = false;      // Mouse selection  object
int trackObject = 0;    // -1 select, 0 init, 1 tracking
Point origin;
cv::Rect2d selection;

// User draws box around object to track. This triggers TrackingAPI's to start tracking
static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject == true )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect2d(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case EVENT_LBUTTONDOWN:
        origin = Point(x, y);
        selection = cv::Rect2d(x, y, 0, 0);
        selectObject = true;
        break;
    case EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
        {
            trackObject = -1;   // Set up Tracking in main() loop
        }
        break;
    default:
        break;
    }
}

const char* keys =
{
    "{help h              |     | show help message}"
    "{@tracking_algorithm | KCF | tracking algorithm    }"
    "{@camera_number      | 0   | camera number    }"
};


int main( int argc, const char** argv )
{
    cv::VideoCapture cap;
    cv::Rect2d track_window;
    cv::CommandLineParser parser(argc, argv, keys);

    // traking algorithm
    cv::String tracking_algorithm = parser.get<cv::String>(0);

    // camera open
    auto cam_num = parser.get<int>(1);
    cap.open(cam_num);

    if(!cap.isOpened())
    {
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        parser.printMessage();
        return -1;
    }

    cv::namedWindow( "OpenCV Tracking API Demo", cv::WINDOW_NORMAL);
    cv::setMouseCallback( "OpenCV Tracking API Demo", onMouse, 0 );

    cv::Ptr<cv::Tracker> tracker;
    auto color_kcf = cv::Scalar(0, 0, 255);

    Mat frame;

    for(;;)
    {
        cap >> frame;
        if( frame.empty() )
        {
            break;
        }

        frame.copyTo(image);

        if(trackObject)
        {

            if(trackObject < 0)
            {
                tracker = cv::Tracker::create(tracking_algorithm);

                track_window = selection;
                trackObject = 1; // Don't set up again, unless user selects new ROI

                tracker->init(image, selection);
            }

            tracker->update(image, selection);

            cv::rectangle(image, selection, color_kcf, 1, 1);
            cv::putText(image, tracking_algorithm, cv::Point(5, 20),
                    cv::FONT_HERSHEY_SIMPLEX, .5, color_kcf, 1, CV_AA);
        }

        // if selected by mouse. Inverts selected object
        if(selectObject && selection.width > 0 && selection.height > 0)
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        imshow( "OpenCV Tracking API Demo", image );

        auto c = (char)waitKey(1);
        if (c == 27)
        {
            break;
        }
    }

    return 0;
}
