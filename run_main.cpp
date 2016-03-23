// imageformatconverter.cpp : Defines the entry point for the console application.
//

#include "RealsenseCam.h"
#include "DrawResult.h"
#include <time.h>
#include <strstream>

using namespace cv;
using namespace std;

//int width1 = 1280;	int height1 = 720;	float fps1 = 30;
int width1 = 640;	int height1 = 480;	float fps1 = 30;
int width2 = 640;	int height2 = 480;	float fps2 = 30;
int width3 = 640;	int height3 = 480;	float fps3 = 30;


PXCCapture::StreamType type1 = PXCCapture::STREAM_TYPE_COLOR;

PXCImage::PixelFormat pixelFormat1 = PXCImage::PIXEL_FORMAT_RGB32;
//PXCImage::PixelFormat pixelFormat1 = PXCImage::PIXEL_FORMAT_RGB24;
//PXCImage::PixelFormat pixelFormat1 = PXCImage::PIXEL_FORMAT_Y8;

PXCCapture::StreamType type2 = PXCCapture::STREAM_TYPE_DEPTH;
//PXCImage::PixelFormat pixelFormat2 = PXCImage::PIXEL_FORMAT_RGB32;
//PXCImage::PixelFormat pixelFormat2 = PXCImage::PIXEL_FORMAT_DEPTH;
PXCImage::PixelFormat pixelFormat2 = PXCImage::PIXEL_FORMAT_DEPTH_F32;


PXCCapture::StreamType type3 = PXCCapture::STREAM_TYPE_IR;
//PXCImage::PixelFormat pixelFormat3 = PXCImage::PIXEL_FORMAT_Y16;
PXCImage::PixelFormat pixelFormat3 = PXCImage::PIXEL_FORMAT_Y8;

int _tmain(int argc, _TCHAR* argv[])
{
	RealsenseCam cam;

	cv::Mat image1;
	cv::Mat image2;
	cv::Mat image3;

	//cam.cameraStreamConfig(type1, width1, height1, fps1);

	cam.cameraStreamConfig(type1, width1, height1, fps1,
		type2, width2, height2, fps2);

	//cam.cameraStreamConfig(type1, width1, height1, fps1,
	//					 type2, width2, height2, fps2,
	//					 type3, width3, height3, fps3);
	
	cam.faceAlgorithmConfig("RecognitionDatabase");
	cam.cameraInit();

	while (true)
	{
		cam.queryFrame();

		//if (!cam.queryImageFromFrame(type1, pixelFormat1, width1, height1, image1))break;

		if (!cam.queryImageFromFrame(type1, pixelFormat1, width1, height1, image1,
			type2, pixelFormat1, width2, height2, image2))break;

		//if (!cam.queryImageFromFrame(type1, pixelFormat1, width1, height1, image1,
		//	type2, pixelFormat2, width2, height2, image2,
		//	type3, pixelFormat3, width3, height3, image3))break;
		
		int nfaces = cam.queryNumberOfFace();

		vector<RealsenseCam::DetectionData> vecDetection;
		cam.queryDetectionData(vecDetection);

		vector<RealsenseCam::LandmarkData> landmark;
		cam.queryLandmarkData(0, landmark);

		RealsenseCam::PoseData pose;
		cam.queryPoseData(0, pose);

		RealsenseCam::ExpressionData expression;
		cam.queryExpressionData(0, expression);

		vector<RealsenseCam::RecognitionData> vecRecognition;
		cam.queryRecognitionData(vecRecognition);

		vector<RealsenseCam::EmotionData> vecEmotion;
		cam.queryEmotionData(vecEmotion);

		//show image
		Mat imageForShow;
		image1.copyTo(imageForShow);

		DrawResult draw;

		//rect
		for (int i = 0; i < nfaces; ++i)
			rectangle(imageForShow, vecDetection[i].faceRect, Scalar(0, 0, 255), 2);
			
		////landmark
		//cam.drawLandmark(imageForShow, landmark);
		
		////eye tracking
		//draw.drawEyeTracking(imageForShow, landmark);

		////expression
		/*if (!vecDetection.empty())
			draw.drawExpression(imageForShow, expression, vecDetection[0].faceRect);*/

		//recognition
		for (int i = 0; i < nfaces; ++i)
			draw.drawRecognition(imageForShow,vecRecognition[i], vecDetection[0].faceRect);

		////emotion
		//for (int i = 0; i < nfaces; ++i)
		//	draw.drawEmotion(imageForShow, vecEmotion[i].emotionLabel, vecEmotion[i].sentimentLabel, vecDetection[i].faceRect);

		//time
		const time_t t = time(NULL);
		tm curTime;
		localtime_s(&curTime, &t);
		string strTime;
		strstream ss;
		ss << curTime.tm_year + 1900 << curTime.tm_mon + 1 << curTime.tm_mday
			<< curTime.tm_hour << curTime.tm_min << curTime.tm_sec;
		ss >> strTime;

		//Mat depth;
		//resize(image2, depth, Size(320,240));
		//Mat colorROI = Mat(imageForShow, Rect(0, 480, depth.cols, depth.rows));
		//depth.copyTo(colorROI);
		imshow("RealsenseCam", imageForShow);
		
		int key = waitKey(33);

		if (key == 32) //space
		{
			imwrite("face\\" + strTime + "image.jpg", image1);
			if (cam.registerUser())
				cout << "Register User." << endl;
		}
		
		if (key == 83) //s
		{
			if (cam.saveRecognitionDatabase("RecognitionDatabase"))
				cout << "Save Database." << endl;
		}


		if (key == 27) //esc
		{
			break;
		}

		cam.releaseFrame();
	}

	return 0;
}

