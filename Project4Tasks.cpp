// Project4Tasks.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <fstream>
#include <cstdint>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <sstream>
#include <math.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


using namespace cv;
using namespace std;

int corners1(cv::Mat& originalimage);

// Driver code
int main(int argc, char** argv)
{

	RNG rng(0xFFFFFFFF);

	int x = 0, captureimagescount = 10; 	// Taking 10 frames for example
	
	while (x < captureimagescount) {

		cv::VideoCapture video(0);
		if (!video.isOpened()) {
			return -1;
			
		}
		cv::Mat frame;

		int frame_width = (int)video.get(cv::CAP_PROP_FRAME_WIDTH);
		int frame_height = (int)video.get(cv::CAP_PROP_FRAME_HEIGHT);
		//cv::VideoWriter output("output.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(frame_width, frame_height));

		video.read(frame);
		if (frame.empty()) {
			printf("frame is empty\n");
			break;
		}

		x++;
		char key = cv::waitKey(25);
		if (key == 'q') {
			break;
		}

		frame.copyTo(image);
		cvtColor(frame, frame, COLOR_BGR2GRAY);
		cv::imshow("Video feed", frame);
		frame.copyTo(gray);



			// Task 1 - Detect and Extract Chessboard Corners                 *****

			Size patternsize(7, 9); //interior number of corners
			Mat img; image.copyTo(img);
			vector<Point2f> corners; //this will be filled by the detected corners
			//CALIB_CB_FAST_CHECK saves a lot of time on images
			//that do not contain any chessboard corners
			bool patternfound = findChessboardCorners(gray, patternsize, corners,
				CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
				+ CALIB_CB_FAST_CHECK);
			if (patternfound) {     // checking whether chess board corners exist in the images.
				cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
					TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
				drawChessboardCorners(img, patternsize, Mat(corners), patternfound);
			}



			//  ********    Task 2 & 3 - Select Calibration Images and caliberate the camera		******

			std::vector<cv::Vec3f> point_set;
			std::vector<std::vector<cv::Vec3f> > point_list;
			std::vector<std::vector<cv::Point2f> > corner_list;

			for (int i = 0; i < 7; i++) {
				for (int j = 0; j < 9; j++) {
					point_set.push_back(cv::Point3f(j, i, 0));
				}
			}

			corner_list.push_back(corners); // push each frame corner into this.
			point_list.push_back(point_set);

			cv::Mat cameraMatrix, distCoeffs, R, T;
			cv::calibrateCamera(point_list, corner_list, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

			cout << "cameraMatrix : " << cameraMatrix << std::endl;
			cout << "distCoeffs : " << distCoeffs << std::endl;
			cout << "Rotation vector : " << R << std::endl;
			cout << "Translation vector : " << T << std::endl;

			cout << "corners" << corner_list[0] << "\n";
			cout << "points" << point_list[0][0] << "\n";



			//  *******		Task 4 - Calculate Current Position of the Camera		*********
			
			vector<Point3f> objectPoints;
			// Mat cameraMatrix1, distCoeffs1; // taken from inside
			Mat rotVec1, transVec1;
			cv::solvePnP(point_set, corners, cameraMatrix, distCoeffs, rotVec1, transVec1);

			std::cout << "cameraMatrix : " << cameraMatrix << std::endl;
			std::cout << "distCoeffs : " << distCoeffs << std::endl;
			std::cout << "Rotation vector : " << rotVec1 << std::endl;
			std::cout << "Translation vector : " << transVec1 << std::endl;



			//	*****	Task 5 - Project Outside Corners or 3D Axes 	******
			
			vector<Point2f> projectedPoints;
			cv::projectPoints(point_set, rotVec1, transVec1, cameraMatrix, distCoeffs, projectedPoints);

			std::cout << "cameraMatrix : " << cameraMatrix << std::endl;
			std::cout << "distCoeffs : " << distCoeffs << std::endl;
			std::cout << "projectedPoints : " << projectedPoints << std::endl;



			// ******	Task 6 - Create a Virtual Object	******
			
			Point pt1, pt2;
			int lineType = 8;
			int thickness = rng.uniform(-3, 10);
			
			Point2f p1 = corners[5];
			Point2f p2 = corners[15];   //select 2 random coordinates by using corners points of chessboard (between corners vector size)

			pt1.x = p1.x; pt1.y = p1.y;
			pt2.x = p2.x; pt2.y = p2.y;
			
			pt1.x = frame_width/2; pt1.y = frame_height/2;
			pt2.x = frame_width/3; pt2.y = frame_height/3;
			
			cout << pt1.x << pt1.y << "\n";
			cout << pt2.x << pt2.y << "\n";
			
			rectangle(img, pt1, pt2, 125, MAX(thickness, -1), lineType);  //we can draw any object like circle, lines , etc by using opencv
			cv::imshow("Video feed1", img);



			// *****	Task 7 - Detect Robust Features		******
			
			corners1(image);

	}

	//imwrite("grayblurred-image.jpg", blur);
	//imshow("Window Name", img);

	// Wait for any keystroke
	//waitKey(0);
	return 0;
}



int corners1(cv::Mat& originalimage) {

	cout << "==============================================" << "\n";

	Mat source, gray;
	// Load source image and convert it to gray
	source = imread("C:/Users/srika/Downloads/test21.png");
	cvtColor(source, gray, cv::COLOR_BGR2GRAY);
	Mat distance, distance_norm, distance_norm_scaled;
	distance = Mat::zeros(source.size(), CV_32FC1);

	// Detecting corners
	cornerHarris(gray, distance, 7, 5, 0.05, BORDER_DEFAULT);

	// Normalizing
	normalize(distance, distance_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(distance_norm, distance_norm_scaled);

	// Drawing a circle around corners
	for (int j = 0; j < distance_norm.rows; j++)
	{
		for (int i = 0; i < distance_norm.cols; i++)
		{
			if ((int)distance_norm.at<float>(j, i) > 200)
			{
				circle(distance_norm_scaled, Point(i, j), 5, Scalar(0), 2, 8, 0);
			}
		}
	}


	// Showing the result
	namedWindow("corners_window", WINDOW_AUTOSIZE);
	imshow("corners_window", distance_norm_scaled);

	waitKey(0);
	return 0;
}