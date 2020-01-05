#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "not video path" << std::endl;
        return -1;
    }

    // create tracker
    cv::Ptr<cv::Tracker> tracker_kcf = cv::Tracker::create("KCF");
    cv::Ptr<cv::Tracker> tracker_tld = cv::Tracker::create("TLD");
    cv::Ptr<cv::Tracker> tracker_median_flow = cv::Tracker::create("MEDIANFLOW");
    cv::Ptr<cv::Tracker> tracker_boosting = cv::Tracker::create("BOOSTING");
    cv::Ptr<cv::Tracker> tracker_mil = cv::Tracker::create("MIL");

    cv::VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        std::cout << "Can't open video" << std::endl;
    }

    cv::Mat video_frame;
    cap >> video_frame;

    // Select tracking target form the first frame.
    auto roi = cv::selectROI("tracker", video_frame);
    auto roi_tld = roi;
    auto roi_median_flow = roi;
    auto roi_boosting = roi;
    auto roi_mil = roi;

    cv::Mat target(video_frame, roi);
    cv::imwrite("target.jpeg", target);
    std::cout << "(x, y, width, height) = (" << roi.x << "," << roi.y << "," <<
        roi.width << "," << roi.height << ")" << std::endl;

    if (roi.width == 0 || roi.height == 0)
    {
        return -1;
    }

    // Initialize tracker.
    tracker_kcf->init(video_frame, roi);
    tracker_tld->init(video_frame, roi_tld);
    tracker_median_flow->init(video_frame, roi_median_flow);
    tracker_boosting->init(video_frame, roi_boosting);
    tracker_mil->init(video_frame, roi_mil);

    // Select color for each tracker
    auto color_kcf = cv::Scalar(0, 255, 0);
    auto color_tld = cv::Scalar(0, 255, 255);
    auto color_median_flow = cv::Scalar(0, 0, 255);
    auto color_boosting = cv::Scalar(255, 255, 0);
    auto color_mil = cv::Scalar(255, 0, 255);

    // Save video settings
    auto fps = cap.get(CV_CAP_PROP_FPS);
    std::cout << "fps :" << fps << std::endl;
    if (fps == 0)
    {
        fps = 30;
    }
    auto size = cv::Size(cap.get(CV_CAP_PROP_FRAME_WIDTH),
                        cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    const auto fourcc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    std::string file_name = "output.avi";
    cv::VideoWriter writer(file_name, fourcc, fps, size);

    while (true)
    {
        cap >> video_frame;
        if (video_frame.empty())
        {
            break;
        }

        // update
        tracker_kcf->update(video_frame, roi);
        tracker_tld->update(video_frame, roi_tld);
        tracker_median_flow->update(video_frame, roi_median_flow);
        tracker_boosting->update(video_frame, roi_boosting);
        tracker_mil->update(video_frame, roi_mil);

        // Surrounded by a rectangle
        cv::rectangle(video_frame, roi, color_kcf, 1, 1);
        cv::rectangle(video_frame, roi_tld, color_tld, 1, 1);
        cv::rectangle(video_frame, roi_median_flow, color_median_flow, 1, 1);
        cv::rectangle(video_frame, roi_boosting, color_boosting, 1, 1);
        cv::rectangle(video_frame, roi_mil, color_mil, 1, 1);

        cv::putText(video_frame, "- KCF", cv::Point(5, 20), cv::FONT_HERSHEY_SIMPLEX, .5, color_kcf, 1, CV_AA);
        cv::putText(video_frame, "- TLD", cv::Point(65, 20), cv::FONT_HERSHEY_SIMPLEX, .5, color_tld, 1, CV_AA);
        cv::putText(video_frame, "- MEDIAN FLOW", cv::Point(125, 20), cv::FONT_HERSHEY_SIMPLEX, .5, color_median_flow, 1, CV_AA);
        cv::putText(video_frame, "- BOOSTING", cv::Point(5, 40), cv::FONT_HERSHEY_SIMPLEX, .5, color_boosting, 1, CV_AA);
        cv::putText(video_frame, "- MIL", cv::Point(65, 40), cv::FONT_HERSHEY_SIMPLEX, .5, color_mil, 1, CV_AA);

        cv::imshow("tracker", video_frame);
        writer << video_frame;

        auto key = cv::waitKey(30);
        if (key == 0x1b)
        {
            break;
        }
    }

    return 0;
}

