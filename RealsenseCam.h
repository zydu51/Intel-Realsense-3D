#pragma once

#include <vector>

//realsense sdk
#include "pxcsensemanager.h"
#include "pxcfacemodule.h"
#include "pxcfacedata.h"
#include "pxcfaceconfiguration.h"

//opencv
#include "opencv2/opencv.hpp"


/**
	prepare
	1.stream and face algorithm config
	2.camera init

	start
	3.get frame
	4.get image and get algorithm data include face alarm, landmark, emotion
		// algorithm design
	5.release frame

	over


*/
const int LANDMARK_ALIGNMENT = -3;

const int NUM_EXPRESSIONS = 22;
const int NUM_EMOTIONS = 10;
const int NUM_PRIMARY_EMOTIONS = 7;

class RealsenseCam
{
public:

	enum EmotionName
	{
		EMOTION_NULL = -1,			//no emotion and sentiment

		EMOTION_PRIMARY_ANGER = 0,	// primary emotion ANGER
		EMOTION_PRIMARY_CONTEMPT = 1,	// primary emotion CONTEMPT
		EMOTION_PRIMARY_DISGUST = 2,	// primary emotion DISGUST
		EMOTION_PRIMARY_FEAR = 3,	// primary emotion FEAR
		EMOTION_PRIMARY_JOY = 4,	// primary emotion JOY
		EMOTION_PRIMARY_SADNESS = 5,	// primary emotion SADNESS
		EMOTION_PRIMARY_SURPRISE = 6,	// primary emotion SURPRISE

		EMOTION_SENTIMENT_POSITIVE = 7,	// Overall sentiment: POSITIVE
		EMOTION_SENTIMENT_NEGATIVE = 8,	// Overall sentiment: NEGATIVE
		EMOTION_SENTIMENT_NEUTRAL = 9,	// Overall sentiment: NEUTRAL
	};

	enum ExpressionName
	{
		EXPRESSION_BROW_RAISER_LEFT = 0,
		EXPRESSION_BROW_RAISER_RIGHT,
		EXPRESSION_BROW_LOWERER_LEFT,
		EXPRESSION_BROW_LOWERER_RIGHT,

		EXPRESSION_SMILE,
		EXPRESSION_KISS,
		EXPRESSION_MOUTH_OPEN,

		EXPRESSION_EYES_CLOSED_LEFT,
		EXPRESSION_EYES_CLOSED_RIGHT,

		EXPRESSION_HEAD_TURN_LEFT,
		EXPRESSION_HEAD_TURN_RIGHT,
		EXPRESSION_HEAD_UP,
		EXPRESSION_HEAD_DOWN,
		EXPRESSION_HEAD_TILT_LEFT,
		EXPRESSION_HEAD_TILT_RIGHT,

		EXPRESSION_EYES_TURN_LEFT,
		EXPRESSION_EYES_TURN_RIGHT,
		EXPRESSION_EYES_UP,
		EXPRESSION_EYES_DOWN,
		EXPRESSION_TONGUE_OUT,
		EXPRESSION_PUFF_RIGHT,
		EXPRESSION_PUFF_LEFT
	};

	struct DetectionData
	{
		cv::Rect faceRect;
		float faceDepth;
	};

	struct LandmarkData
	{
		cv::Point2f imagePoint;
		cv::Point3f worldPoint;
		int confidenceImage;
		int confidenceWorld;
	};

	struct EulerAngles
	{
		float yaw;
		float pitch;
		float roll;
	};
	struct PoseData
	{
		cv::Point3f headPosition;
		EulerAngles angles;
		int confidence;
	};

	struct EmotionData
	{
		EmotionName emotionLabel;
		EmotionName sentimentLabel;
		float evidence[NUM_EMOTIONS];
		float intensity[NUM_EMOTIONS];
	};
	
	struct ExpressionData
	{
		int intensity[NUM_EXPRESSIONS];
	};

	struct RecognitionData
	{
		int UserID;
		bool isRegistered;
	};

	RealsenseCam();
	~RealsenseCam();
	
	//stream
	pxcStatus cameraStreamConfig(PXCCapture::StreamType streamType, int width, int height, float fps);

	pxcStatus cameraStreamConfig(PXCCapture::StreamType streamType1, int width1, int height1, float fps1,
							   PXCCapture::StreamType streamType2, int width2, int height2, float fps2);

	pxcStatus cameraStreamConfig(PXCCapture::StreamType streamType1, int width1, int height1, float fps1,
							   PXCCapture::StreamType streamType2, int width2, int height2, float fps2,
							   PXCCapture::StreamType streamType3, int width3, int height3, float fps3);
	
	//face
	pxcStatus faceAlgorithmConfig(std::string strRecognitionDatabaseFileName);

	//init
	pxcStatus cameraInit();

	//frame
	pxcStatus queryFrame();
	
	//image
	bool queryImageFromFrame(PXCCapture::StreamType streamType, PXCImage::PixelFormat pixelFormat, int width, int height, cv::Mat &imageMat);
	
	bool queryImageFromFrame(PXCCapture::StreamType streamType1, PXCImage::PixelFormat pixelFormat1, int width1, int height1, cv::Mat &imageMat1,
		PXCCapture::StreamType streamType2, PXCImage::PixelFormat pixelFormat2, int width2, int height2, cv::Mat &imageMat2);
	
	bool queryImageFromFrame(PXCCapture::StreamType streamType1, PXCImage::PixelFormat pixelFormat1, int width1, int height1, cv::Mat &imageMat1,
					PXCCapture::StreamType streamType2, PXCImage::PixelFormat pixelFormat2, int width2, int height2, cv::Mat &imageMat2,
					PXCCapture::StreamType streamType3, PXCImage::PixelFormat pixelFormat3, int width3, int height3, cv::Mat &imageMat3);
	
	//face number
	int queryNumberOfFace();

	//detection
	bool queryDetectionData(int faceIdx, DetectionData &detectionData);

	bool queryDetectionData(std::vector<DetectionData> &vecDetectionData);

	//landmark
	int queryNumberOfLandmark();

	bool queryLandmarkData(int faceIdx, std::vector<LandmarkData> &landmarkData);

	//NOTICE!!!!!!!!!!!!!!!!!!!!!!!!!!
	//only the index 0 face has landmark data
	//so this function is useless for now
	bool queryLandmarkData(std::vector<std::vector<LandmarkData>> &vecLandmarkData);

	//pose
	bool queryPoseData(int faceIdx, PoseData &poseData);

	//NOTICE!!!!!!!!!!!!!!!!!!!!!!!!!!
	//only the index 0 face has pose data
	//so this function is useless for now
	bool queryPoseData(std::vector<PoseData> &vecPoseData);

	//expression
	bool queryExpressionData(int faceIdx, ExpressionData &expressionData);

	//NOTICE!!!!!!!!!!!!!!!!!!!!!!!!!!
	//only the index 0 face has expression data
	//so this function is useless for now
	bool queryExpressionData(std::vector<ExpressionData> &vecExpressionData);
	
	//recognition
	bool queryRecognitionData(int faceIdx, RecognitionData &recognitionData);

	bool queryRecognitionData(std::vector<RecognitionData> &vecRecognitionData);

	bool registerUser();

	bool saveRecognitionDatabase(std::string strFileName);

	//emotion
	bool queryEmotionData(int faceID, EmotionData &emotionData);

	bool queryEmotionData(std::vector<EmotionData> &vecEmotionData);

	void releaseFrame();

protected:
	PXCSenseManager *senseManager;
	PXCFaceData *faceData;

	int pixelFormatConvertRssdkToOpencv(PXCImage::PixelFormat pixelFormat);

	int landmarksNumber;
	//PXCFaceData::LandmarkPoint *landmarkPoints;

	const double FACEOCCLUDED_THRESHHOLD = 0.5;

};

