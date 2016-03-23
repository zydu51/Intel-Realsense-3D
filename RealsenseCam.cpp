#include "stdafx.h"
#include "RealsenseCam.h"
#include <fstream>


RealsenseCam::RealsenseCam()
{
	senseManager = PXCSenseManager::CreateInstance();
	//landmarkPoints = NULL;
}

RealsenseCam::~RealsenseCam()
{
	//if (landmarkPoints != NULL)
	//	delete[] landmarkPoints;
	senseManager->Release();
}

pxcStatus RealsenseCam::cameraStreamConfig(PXCCapture::StreamType streamType, int width, int height, float fps)
{
	pxcStatus status = PXC_STATUS_NO_ERROR;
	status = senseManager->EnableStream(streamType, width, height, fps);
	if (status < PXC_STATUS_NO_ERROR) return status;
	status = senseManager->Init();
	if (status < PXC_STATUS_NO_ERROR) return status;
	return status;
}

pxcStatus RealsenseCam::cameraStreamConfig(PXCCapture::StreamType streamType1, int width1, int height1, float fps1,
										PXCCapture::StreamType streamType2, int width2, int height2, float fps2)
{
	pxcStatus status = PXC_STATUS_NO_ERROR;
	status = senseManager->EnableStream(streamType1, width1, height1, fps1);
	if (status < PXC_STATUS_NO_ERROR) return status;
	status = senseManager->EnableStream(streamType2, width2, height2, fps2);
	if (status < PXC_STATUS_NO_ERROR) return status;
	status = senseManager->Init();
	if (status < PXC_STATUS_NO_ERROR) return status;
	return status;
}

pxcStatus RealsenseCam::cameraStreamConfig(PXCCapture::StreamType streamType1, int width1, int height1, float fps1,
										 PXCCapture::StreamType streamType2, int width2, int height2, float fps2,
										 PXCCapture::StreamType streamType3, int width3, int height3, float fps3)
{
	pxcStatus status = PXC_STATUS_NO_ERROR;
	status = senseManager->EnableStream(streamType1, width1, height1, fps1);
	if (status < PXC_STATUS_NO_ERROR) return status;
	status = senseManager->EnableStream(streamType2, width2, height2, fps2);
	if (status < PXC_STATUS_NO_ERROR) return status;
	status = senseManager->EnableStream(streamType3, width3, height3, fps3);
	if (status < PXC_STATUS_NO_ERROR) return status;
	status = senseManager->Init();
	if (status < PXC_STATUS_NO_ERROR) return status;
	return status;
}

pxcStatus RealsenseCam::faceAlgorithmConfig(std::string strRecognitionDatabaseFileName)
{
	//emotion algorithm
	pxcStatus status = senseManager->EnableEmotion();
	if (status < PXC_STATUS_NO_ERROR) return status;

	//face algorithm include detection, landmark, pose, expression, pulse, alert, etc.
	status = senseManager->EnableFace();
	if (status < PXC_STATUS_NO_ERROR) return status;

	PXCFaceModule *face = senseManager->QueryFace();
	PXCFaceConfiguration *cfg = face->CreateActiveConfiguration();

	//face tracking mode
	//cfg->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR);
	cfg->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);

	//detection
	cfg->detection.isEnabled = true;
	
	//landmark
	cfg->landmarks.isEnabled = true;
	landmarksNumber = cfg->landmarks.numLandmarks;
	
	//pose
	cfg->pose.isEnabled = true;
	
	//expression
	cfg->QueryExpressions()->Enable();
	cfg->QueryExpressions()->EnableAllExpressions();
	
	//pulse
	//cfg->QueryPulse()->Enable();
	
	//alert
	//cfg->EnableAllAlerts();

	//recognition
	PXCFaceConfiguration::RecognitionConfiguration *rcfg = cfg->QueryRecognition();
	rcfg->Enable();
	PXCFaceConfiguration::RecognitionConfiguration::RecognitionStorageDesc desc = {};
	desc.maxUsers = 100;
	rcfg->CreateStorage(L"MyDB", &desc);
	rcfg->UseStorage(L"MyDB");
	rcfg->SetRegistrationMode(PXCFaceConfiguration::RecognitionConfiguration::REGISTRATION_MODE_CONTINUOUS);
	
	std::fstream file(strRecognitionDatabaseFileName, std::ios::in | std::ios::binary);
	if (!file)	return PXC_STATUS_FILE_READ_FAILED;
	file.seekg(0, std::ios::end);	//seek for end
	int len = file.tellg();			//get file len
	file.seekg(0, std::ios::beg);	//seek back
	pxcBYTE *buffer = new pxcBYTE[len];
	file.read((char*)buffer, len);
	file.close();
	rcfg->SetDatabaseBuffer(buffer, len);
	delete[] buffer;

	status = cfg->ApplyChanges();
	faceData = face->CreateOutput();

	return PXC_STATUS_NO_ERROR;
}

pxcStatus RealsenseCam::cameraInit()
{
	return senseManager->Init();
}

pxcStatus RealsenseCam::queryFrame()
{
	return senseManager->AcquireFrame(true);
}

bool RealsenseCam::queryImageFromFrame(PXCCapture::StreamType streamType,PXCImage::PixelFormat pixelFormat, int width, int height, cv::Mat &imageMat)
{
	PXCCapture::Sample *sample = senseManager->QuerySample();
	PXCImage::ImageData imageData;
	
	PXCImage *image;
	switch (streamType)
	{
	case PXCCapture::STREAM_TYPE_COLOR:
		image = sample->color;
		break;
	case PXCCapture::STREAM_TYPE_DEPTH:
		image = sample->depth;
		break;
	case PXCCapture::STREAM_TYPE_IR:
		image = sample->ir;
		break;
	default:
		return false;
	}
	image->AcquireAccess(PXCImage::ACCESS_READ, pixelFormat, &imageData);

	if (-1 == pixelFormatConvertRssdkToOpencv(pixelFormat))return false;
	cv::Mat temp(height, width, pixelFormatConvertRssdkToOpencv(pixelFormat), imageData.planes[0]);
	
	//cv::imshow("for debug",temp);
	
	temp.copyTo(imageMat);
	image->ReleaseAccess(&imageData);

	return true;
}

bool RealsenseCam::queryImageFromFrame(PXCCapture::StreamType streamType1, PXCImage::PixelFormat pixelFormat1, int width1, int height1, cv::Mat &imageMat1,
							  PXCCapture::StreamType streamType2, PXCImage::PixelFormat pixelFormat2, int width2, int height2, cv::Mat &imageMat2)
{
	PXCCapture::Sample *sample = senseManager->QuerySample();
	PXCImage *image1;
	PXCImage *image2;
	PXCImage::ImageData imageData1;
	PXCImage::ImageData imageData2;
	
	//image1
	switch (streamType1)
	{
	case PXCCapture::STREAM_TYPE_COLOR:
		image1 = sample->color;
		break;
	case PXCCapture::STREAM_TYPE_DEPTH:
		image1 = sample->depth;
		break;
	case PXCCapture::STREAM_TYPE_IR:
		image1 = sample->ir;
		break;
	default:
		return false;
	}
	image1->AcquireAccess(PXCImage::ACCESS_READ, pixelFormat1, &imageData1);
	if (-1 == pixelFormatConvertRssdkToOpencv(pixelFormat1))return false;
	cv::Mat temp1(height1, width1, pixelFormatConvertRssdkToOpencv(pixelFormat1), imageData1.planes[0]);
	temp1.copyTo(imageMat1);
	image1->ReleaseAccess(&imageData1);

	//image2
	switch (streamType2)
	{
	case PXCCapture::STREAM_TYPE_COLOR:
		image2 = sample->color;
		break;
	case PXCCapture::STREAM_TYPE_DEPTH:
		image2 = sample->depth;
		break;
	case PXCCapture::STREAM_TYPE_IR:
		image2 = sample->ir;
		break;
	default:
		return false;
	}
	image2->AcquireAccess(PXCImage::ACCESS_READ, pixelFormat2, &imageData2);
	if (-1 == pixelFormatConvertRssdkToOpencv(pixelFormat2))return false;
	cv::Mat temp2(height2, width2, pixelFormatConvertRssdkToOpencv(pixelFormat2), imageData2.planes[0]);
	temp2.copyTo(imageMat2);
	image2->ReleaseAccess(&imageData2);

	return true;
}

bool RealsenseCam::queryImageFromFrame(PXCCapture::StreamType streamType1, PXCImage::PixelFormat pixelFormat1, int width1, int height1, cv::Mat &imageMat1,
							  PXCCapture::StreamType streamType2, PXCImage::PixelFormat pixelFormat2, int width2, int height2, cv::Mat &imageMat2,
							  PXCCapture::StreamType streamType3, PXCImage::PixelFormat pixelFormat3, int width3, int height3, cv::Mat &imageMat3)
{

	PXCCapture::Sample *sample = senseManager->QuerySample();
	PXCImage *image1;
	PXCImage *image2;
	PXCImage *image3;
	PXCImage::ImageData imageData1;
	PXCImage::ImageData imageData2;
	PXCImage::ImageData imageData3;

	//image1
	switch (streamType1)
	{
	case PXCCapture::STREAM_TYPE_COLOR:
		image1 = sample->color;
		break;
	case PXCCapture::STREAM_TYPE_DEPTH:
		image1 = sample->depth;
		break;
	case PXCCapture::STREAM_TYPE_IR:
		image1 = sample->ir;
		break;
	default:
		return false;
	}
	image1->AcquireAccess(PXCImage::ACCESS_READ, pixelFormat1, &imageData1);
	if (-1 == pixelFormatConvertRssdkToOpencv(pixelFormat1))return false;
	cv::Mat temp1(height1, width1, pixelFormatConvertRssdkToOpencv(pixelFormat1), imageData1.planes[0]);
	temp1.copyTo(imageMat1);
	image1->ReleaseAccess(&imageData1);

	//image2
	switch (streamType2)
	{
	case PXCCapture::STREAM_TYPE_COLOR:
		image2 = sample->color;
		break;
	case PXCCapture::STREAM_TYPE_DEPTH:
		image2 = sample->depth;
		break;
	case PXCCapture::STREAM_TYPE_IR:
		image2 = sample->ir;
		break;
	default:
		return false;
	}
	image2->AcquireAccess(PXCImage::ACCESS_READ, pixelFormat2, &imageData2);
	if (-1 == pixelFormatConvertRssdkToOpencv(pixelFormat2))return false;
	cv::Mat temp2(height2, width2, pixelFormatConvertRssdkToOpencv(pixelFormat2), imageData2.planes[0]);
	temp2.copyTo(imageMat2);
	image2->ReleaseAccess(&imageData2);
	
	//image3
	switch (streamType3)
	{
	case PXCCapture::STREAM_TYPE_COLOR:
		image3 = sample->color;
		break;
	case PXCCapture::STREAM_TYPE_DEPTH:
		image3 = sample->depth;
		break;
	case PXCCapture::STREAM_TYPE_IR:
		image3 = sample->ir;
		break;
	default:
		return false;
	}
	image3->AcquireAccess(PXCImage::ACCESS_READ, pixelFormat3, &imageData3);
	if (-1 == pixelFormatConvertRssdkToOpencv(pixelFormat3))return false;
	cv::Mat temp3(height3, width3, pixelFormatConvertRssdkToOpencv(pixelFormat3), imageData3.planes[0]);
	temp3.copyTo(imageMat3);
	image3->ReleaseAccess(&imageData3);

	return true;
}


int RealsenseCam::queryNumberOfFace()
{
	faceData->Update();
	return faceData->QueryNumberOfDetectedFaces();
}

bool RealsenseCam::queryDetectionData(int faceIdx, DetectionData &detectionData)
{
	//clear for new data
	detectionData.faceDepth = 0.0;
	detectionData.faceRect = cv::Rect(0, 0, 0, 0);

	//update face data
	faceData->Update();
	
	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0 || faceIdx > nfaces - 1)	return false;
	
	//query face data
	PXCFaceData::Face *face = faceData->QueryFaceByIndex(faceIdx);
	if (face == NULL)	return false;

	//query detection data
	PXCFaceData::DetectionData *pxcDetectionData = face->QueryDetection();
	if (pxcDetectionData == NULL)	return false;
	
	//depth
	pxcDetectionData->QueryFaceAverageDepth(&detectionData.faceDepth);
	
	//rect
	PXCRectI32 rect;
	pxcDetectionData->QueryBoundingRect(&rect);
	detectionData.faceRect = cv::Rect(rect.x, rect.y, rect.w, rect.h);
	
	return true;
}

bool RealsenseCam::queryDetectionData(std::vector<DetectionData> &vecDetectionData)
{
	//clear the vector for new data
	vecDetectionData.clear();
	
	//update face data
	faceData->Update();
	
	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;
	
	DetectionData detectionData;
	for (int i = 0; i < nfaces; ++i)
	{
		//query face data
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(i);
		if (face == NULL)
		{
			detectionData.faceDepth = 0.0;
			detectionData.faceRect = cv::Rect(0, 0, 0, 0);
			vecDetectionData.push_back(detectionData);
			continue;
		}

		//query detection data
		PXCFaceData::DetectionData *pxcDetectionData = face->QueryDetection();
		if (pxcDetectionData == NULL)
		{
			detectionData.faceDepth = 0.0;
			detectionData.faceRect = cv::Rect(0, 0, 0, 0);
			vecDetectionData.push_back(detectionData);
			continue;
		}

		//depth
		pxcDetectionData->QueryFaceAverageDepth(&detectionData.faceDepth);

		//rect
		PXCRectI32 rect;
		pxcDetectionData->QueryBoundingRect(&rect);
		detectionData.faceRect = cv::Rect(rect.x, rect.y, rect.w, rect.h);
		
		vecDetectionData.push_back(detectionData);
	}
	return true;
}

int RealsenseCam::queryNumberOfLandmark()
{
	return landmarksNumber;
}

bool RealsenseCam::queryLandmarkData(int faceIdx, std::vector<LandmarkData> &landmarkData)
{
	//clear landmark vector for data
	landmarkData.clear();

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0 || faceIdx > nfaces - 1)	return false;

	//query face data
	PXCFaceData::Face *face = faceData->QueryFaceByIndex(faceIdx);
	if (face == NULL)	return false;

	//query landmark data
	PXCFaceData::LandmarksData *pxclandmarkData = face->QueryLandmarks();
	if (pxclandmarkData == NULL)	return false;

	pxcI32 numPoints = pxclandmarkData->QueryNumPoints();
	PXCFaceData::LandmarkPoint *landmarkPoints = new PXCFaceData::LandmarkPoint[numPoints];
	pxclandmarkData->QueryPoints(landmarkPoints);
	LandmarkData tempLandmarkpoint;
	for (int i = 0; i < numPoints; ++i)
	{
		tempLandmarkpoint.imagePoint = cv::Point2f(landmarkPoints[i].image.x, landmarkPoints[i].image.y);
		tempLandmarkpoint.worldPoint = cv::Point3f(landmarkPoints[i].world.x, landmarkPoints[i].world.y, landmarkPoints[i].world.z);
		tempLandmarkpoint.confidenceImage = landmarkPoints[i].confidenceImage;
		tempLandmarkpoint.confidenceWorld = landmarkPoints[i].confidenceWorld;
		landmarkData.push_back(tempLandmarkpoint);
	}
	delete[]landmarkPoints;
	return true;
}

bool RealsenseCam::queryLandmarkData(std::vector<std::vector<LandmarkData>> &vecLandmarkData)
{
	//clear landmark vector for data
	vecLandmarkData.clear();

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;

	std::vector<LandmarkData> landmarkData;
	for (int i = 0; i < nfaces; i++)
	{
		//query face data
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(i);
		if (face == NULL)
		{
			landmarkData.clear();
			vecLandmarkData.push_back(landmarkData);
			continue;
		}

		//query landmark data
		PXCFaceData::LandmarksData *pxclandmarkData = face->QueryLandmarks();
		if (pxclandmarkData == NULL)
		{
			landmarkData.clear();
			vecLandmarkData.push_back(landmarkData);
			continue;
		}
		pxcI32 numPoints = pxclandmarkData->QueryNumPoints();
		PXCFaceData::LandmarkPoint *landmarkPoints = new PXCFaceData::LandmarkPoint[numPoints];
		pxclandmarkData->QueryPoints(landmarkPoints);
		LandmarkData tempLandmarkpoint;
		for (int i = 0; i < numPoints; ++i)
		{
			tempLandmarkpoint.imagePoint = cv::Point2f(landmarkPoints[i].image.x, landmarkPoints[i].image.y);
			tempLandmarkpoint.worldPoint = cv::Point3f(landmarkPoints[i].world.x, landmarkPoints[i].world.y, landmarkPoints[i].world.z);
			tempLandmarkpoint.confidenceImage = landmarkPoints[i].confidenceImage;
			tempLandmarkpoint.confidenceWorld = landmarkPoints[i].confidenceWorld;
			landmarkData.push_back(tempLandmarkpoint);
		}
		delete[]landmarkPoints;
		
		vecLandmarkData.push_back(landmarkData);
	}
	return true;
}

bool RealsenseCam::queryPoseData(int faceIdx, PoseData &poseData)
{
	//clear for new data
	poseData.headPosition = cv::Point3f(0.0, 0.0, 0.0);
	poseData.confidence = 0;
	poseData.angles.yaw = 0.0; poseData.angles.pitch = 0.0; poseData.angles.roll= 0.0;

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0 || faceIdx > nfaces - 1)	return false;

	//query face data
	PXCFaceData::Face *face = faceData->QueryFaceByIndex(faceIdx);
	if (face == NULL)	return false;

	//query pose data
	PXCFaceData::PoseData *pxcPoseData = face->QueryPose();
	if (pxcPoseData == NULL)	return false;

	//headCenter
	PXCFaceData::HeadPosition headPosition;
	pxcPoseData->QueryHeadPosition(&headPosition);
	poseData.headPosition = cv::Point3f(headPosition.headCenter.x, headPosition.headCenter.y, headPosition.headCenter.z);

	//angle
	PXCFaceData::PoseEulerAngles angle;
	pxcPoseData->QueryPoseAngles(&angle);
	poseData.angles.yaw = angle.yaw;
	poseData.angles.roll = angle.roll;
	poseData.angles.pitch = angle.pitch;

	//confidence
	poseData.confidence = pxcPoseData->QueryConfidence();

	return true;
}

bool RealsenseCam::queryPoseData(std::vector<PoseData> &vecPoseData)
{
	//clear for new data
	vecPoseData.clear();

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;

	PoseData poseData;
	for (int i = 0; i < nfaces; i++)
	{
		//query face data
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(i);
		if (face == NULL)
		{
			poseData.headPosition = cv::Point3f(0.0, 0.0, 0.0);
			poseData.confidence = 0;
			poseData.angles.yaw = 0.0; poseData.angles.pitch = 0.0; poseData.angles.roll = 0.0;
			vecPoseData.push_back(poseData);
			continue;
		}

		//query pose data
		PXCFaceData::PoseData *pxcPoseData = face->QueryPose();
		if (pxcPoseData == NULL)
		{
			poseData.headPosition = cv::Point3f(0.0, 0.0, 0.0);
			poseData.confidence = 0;
			poseData.angles.yaw = 0.0; poseData.angles.pitch = 0.0; poseData.angles.roll = 0.0;
			vecPoseData.push_back(poseData);
			continue;
		}

		//headCenter
		PXCFaceData::HeadPosition headPosition;
		pxcPoseData->QueryHeadPosition(&headPosition);
		poseData.headPosition = cv::Point3f(headPosition.headCenter.x, headPosition.headCenter.y, headPosition.headCenter.z);

		//angle
		PXCFaceData::PoseEulerAngles angle;
		pxcPoseData->QueryPoseAngles(&angle);
		poseData.angles.yaw = angle.yaw;
		poseData.angles.roll = angle.roll;
		poseData.angles.pitch = angle.pitch;

		//confidence
		poseData.confidence = pxcPoseData->QueryConfidence();

		vecPoseData.push_back(poseData);
	}
	return true;
}

bool RealsenseCam::queryExpressionData(int faceIdx, ExpressionData &expressionData)
{
	//clear for new data
	memset(expressionData.intensity, -1, sizeof(expressionData.intensity));

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0 || faceIdx > nfaces - 1)	return false;

	//query face data
	PXCFaceData::Face *face = faceData->QueryFaceByIndex(faceIdx);
	if (face == NULL)	return false;

	//query expression data
	PXCFaceData::ExpressionsData* pxcExpressionData = face->QueryExpressions();
	if (pxcExpressionData == NULL)	return false;

	//intensity
	for (int i = 0; i < NUM_EXPRESSIONS; ++i)
	{
		PXCFaceData::ExpressionsData::FaceExpressionResult expressionResult;
		if (pxcExpressionData->QueryExpression((PXCFaceData::ExpressionsData::FaceExpression)ExpressionName(i), &expressionResult))
			expressionData.intensity[i] = expressionResult.intensity;
	}

	return true;
}

bool RealsenseCam::queryExpressionData(std::vector<ExpressionData> &vecExpressionData)
{
	//clear for new data
	vecExpressionData.clear();

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;

	ExpressionData expressionData;
	for (int i = 0; i < nfaces; i++)
	{
		//clear for new data
		memset(expressionData.intensity, -1, sizeof(expressionData.intensity));
		
		//query face data
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(i);
		if (face == NULL)
		{
			vecExpressionData.push_back(expressionData);
			continue;
		}

		//query expression data
		PXCFaceData::ExpressionsData* pxcExpressionData = face->QueryExpressions();
		if (pxcExpressionData == NULL)		
		{
			vecExpressionData.push_back(expressionData);
			continue;
		}

		//intensity
		for (int i = 0; i < NUM_EXPRESSIONS; ++i)
		{
			PXCFaceData::ExpressionsData::FaceExpressionResult expressionResult;
			if (pxcExpressionData->QueryExpression((PXCFaceData::ExpressionsData::FaceExpression)ExpressionName(i), &expressionResult))
				expressionData.intensity[i] = expressionResult.intensity;
		}

		vecExpressionData.push_back(expressionData);
	}
	return true;
}

bool RealsenseCam::queryRecognitionData(int faceIdx, RecognitionData &recognitionData)
{
	//clear for new data
	recognitionData.UserID = -1;
	recognitionData.isRegistered = false;

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0 || faceIdx > nfaces - 1)	return false;

	//query face data
	PXCFaceData::Face *face = faceData->QueryFaceByIndex(faceIdx);
	if (face == NULL)	return false;

	//query recognition data
	PXCFaceData::RecognitionData *pxcRecognitionData = face->QueryRecognition();
	if (pxcRecognitionData == NULL)	return false;

	//UserID
	recognitionData.UserID = pxcRecognitionData->QueryUserID();

	//isRegistered
	recognitionData.isRegistered = pxcRecognitionData->IsRegistered();

	return true;
}

bool RealsenseCam::queryRecognitionData(std::vector<RecognitionData> &vecRecognitionData)
{
	//clear the vector for new data
	vecRecognitionData.clear();

	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;

	RecognitionData recognitionData;
	for (int i = 0; i < nfaces; ++i)
	{
		//query face data
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(i);
		if (face == NULL)
		{
			recognitionData.UserID = -1;
			recognitionData.isRegistered = false;
			vecRecognitionData.push_back(recognitionData);
			continue;
		}

		//query recognition data
		PXCFaceData::RecognitionData *pxcRecognitionData = face->QueryRecognition();
		if (pxcRecognitionData == NULL)
		{
			recognitionData.UserID = -1;
			recognitionData.isRegistered = false;
			vecRecognitionData.push_back(recognitionData);
			continue;
		}

		//UserID
		recognitionData.UserID = pxcRecognitionData->QueryUserID();

		//isRegistered
		recognitionData.isRegistered = pxcRecognitionData->IsRegistered();

		vecRecognitionData.push_back(recognitionData);
	}
	return true;
}

bool RealsenseCam::registerUser()
{
	//update face data
	faceData->Update();

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;

	for (int i = 0; i < nfaces; ++i)
	{
		//query face data
		PXCFaceData::Face *face = faceData->QueryFaceByIndex(i);
		if (face == NULL)	continue;

		//query detection data
		PXCFaceData::RecognitionData *pxcRecognitionData = face->QueryRecognition();
		if (pxcRecognitionData == NULL)		continue;

		//if (pxcRecognitionData->IsRegistered()) continue;

		pxcRecognitionData->RegisterUser();
	}
	return true;
}

bool RealsenseCam::saveRecognitionDatabase(std::string strFileName)
{
	faceData->Update();

	std::fstream file(strFileName,std::ios::out | std::ios::binary);
	if (!file.is_open())	return false;

	PXCFaceData::RecognitionModuleData *rmd = faceData->QueryRecognitionModule();
	
	pxcI32 nbytes = rmd->QueryDatabaseSize();
	pxcBYTE *buffer = new pxcBYTE[nbytes];
	rmd->QueryDatabaseBuffer(buffer);
	file.write((char*)buffer, nbytes);
	file.close();
	delete[] buffer;

	//std::fstream file1(strFileName, std::ios::in | std::ios::binary);
	//if (!file1)	return PXC_STATUS_FILE_READ_FAILED;
	//file1.seekg(0, std::ios::end);	//seek for end
	//int len1 = file1.tellg();			//get file len
	//file1.seekg(0, std::ios::beg);	//seek back
	//pxcBYTE *buffer1 = new pxcBYTE[len1];
	//file.read((char*)buffer1, len1);
	//file.close();
	//rcfg->SetDatabaseBuffer(buffer1, len1);
	//delete[] buffer1;

	return true;
}

bool RealsenseCam::queryEmotionData(int faceID, EmotionData &emotionData)
{
	//clear for new data
	emotionData.emotionLabel = EMOTION_NULL;
	emotionData.sentimentLabel = EMOTION_NULL;
	memset(emotionData.evidence, 0, sizeof(emotionData.evidence));
	memset(emotionData.intensity, 0, sizeof(emotionData.intensity));

	//query emotion
	PXCEmotion *emotionDet = senseManager->QueryEmotion();
	if (emotionDet == NULL) return false;

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0 || faceID > nfaces - 1)	return false;

	PXCEmotion::EmotionData arrData[NUM_EMOTIONS];
	emotionDet->QueryAllEmotionData(faceID, &arrData[0]);

	//get evidence & intensity
	for (int i = 0; i < NUM_EMOTIONS; ++i)
	{
		emotionData.evidence[i] = arrData[i].evidence;
		emotionData.intensity[i] = arrData[i].intensity;
	}

	emotionData.emotionLabel = EMOTION_NULL;
	emotionData.sentimentLabel = EMOTION_NULL;
	
	//calculate emotion label
	int epidx = -1; pxcI32 maxscoreE = -3; pxcF32 maxscoreI = 0;
	for (int i = 0; i < NUM_PRIMARY_EMOTIONS; i++)
	{
		if (emotionData.evidence[i] < maxscoreE)  continue;
		if (emotionData.intensity[i] < maxscoreI) continue;
		maxscoreE = emotionData.evidence[i];
		maxscoreI = emotionData.intensity[i];
		epidx = i;
	}
	if (epidx == -1 || maxscoreI < 0.4)	return false;
	emotionData.emotionLabel = EmotionName(epidx);

	//calculate sentiment label
	int spidx = -1;	maxscoreE = -3; maxscoreI = 0;
	for (int i = 0; i < (NUM_EMOTIONS - NUM_PRIMARY_EMOTIONS); i++) {
		if (emotionData.evidence[NUM_PRIMARY_EMOTIONS + i]< maxscoreE) continue;
		if (emotionData.intensity[NUM_PRIMARY_EMOTIONS + i] < maxscoreI) continue;
		maxscoreE = emotionData.evidence[NUM_PRIMARY_EMOTIONS + i];
		maxscoreI = emotionData.intensity[NUM_PRIMARY_EMOTIONS + i];
		spidx = i;
	}
	if (spidx == -1)	return false;

	emotionData.sentimentLabel = EmotionName(NUM_PRIMARY_EMOTIONS + spidx);

	return true;
}

bool RealsenseCam::queryEmotionData(std::vector<EmotionData> &vecEmotionData)
{
	//clear for new data
	vecEmotionData.clear();

	//query emotion
	PXCEmotion *emotionDet = senseManager->QueryEmotion();
	if (emotionDet == NULL) return false;

	//query face number
	pxcI32 nfaces = faceData->QueryNumberOfDetectedFaces();
	if (nfaces == 0)	return false;

	EmotionData emotionData;
	for (int i = 0; i < nfaces; i++)
	{
		PXCEmotion::EmotionData arrData[NUM_EMOTIONS];
		emotionDet->QueryAllEmotionData(i, &arrData[0]);

		//get evidence & intensity
		for (int i = 0; i < NUM_EMOTIONS; ++i)
		{
			emotionData.evidence[i] = arrData[i].evidence;
			emotionData.intensity[i] = arrData[i].intensity;
		}

		emotionData.emotionLabel = EMOTION_NULL;
		emotionData.sentimentLabel = EMOTION_NULL;

		//calculate emotion label
		int epidx = -1; pxcI32 maxscoreE = -3; pxcF32 maxscoreI = 0;
		for (int i = 0; i < NUM_PRIMARY_EMOTIONS; i++)
		{
			if (emotionData.evidence[i] < maxscoreE)  continue;
			if (emotionData.intensity[i] < maxscoreI) continue;
			maxscoreE = emotionData.evidence[i];
			maxscoreI = emotionData.intensity[i];
			epidx = i;
		}
		if (epidx == -1 || maxscoreI < 0.4)
		{
			vecEmotionData.push_back(emotionData);
			continue;
		}
		emotionData.emotionLabel = EmotionName(epidx);

		//calculate sentiment label
		int spidx = -1;	maxscoreE = -3; maxscoreI = 0;
		for (int i = 0; i < (NUM_EMOTIONS - NUM_PRIMARY_EMOTIONS); i++) {
			if (emotionData.evidence[NUM_PRIMARY_EMOTIONS + i] < maxscoreE) continue;
			if (emotionData.intensity[NUM_PRIMARY_EMOTIONS + i] < maxscoreI) continue;
			maxscoreE = emotionData.evidence[NUM_PRIMARY_EMOTIONS + i];
			maxscoreI = emotionData.intensity[NUM_PRIMARY_EMOTIONS + i];
			spidx = i;
		}
		if (spidx == -1)
		{
			vecEmotionData.push_back(emotionData);
			continue;
		}
		emotionData.sentimentLabel = EmotionName(NUM_PRIMARY_EMOTIONS + spidx);

		vecEmotionData.push_back(emotionData);
	}
	return true;
}

void RealsenseCam::releaseFrame()
{
	senseManager->ReleaseFrame();
}

int RealsenseCam::pixelFormatConvertRssdkToOpencv(PXCImage::PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
		//color camera
	case PXCImage::PIXEL_FORMAT_RGB32:			/* BGRA layout on a little-endian machine */
		return CV_8UC4;
		break;
	case PXCImage::PIXEL_FORMAT_RGB24:			/* BGR layout on a little-endian machine */
		return CV_8UC3;
		break;


		//depth camera
	case PXCImage::PIXEL_FORMAT_DEPTH:			/* 16-bit unsigned integer with precision mm. */
		return CV_16UC1;
		break;
	case PXCImage::PIXEL_FORMAT_DEPTH_F32:		/* 32-bit float-point with precision mm. */
		return CV_32FC1;
		break;

		//ir camera
	case PXCImage::PIXEL_FORMAT_Y16:			/* 16-Bit Gray Image */
		return CV_16UC1;
		break;

		// ir & color camera
	case PXCImage::PIXEL_FORMAT_Y8:				/* 8-Bit Gray Image, or IR 8-bit */
		return CV_8UC1;
		break;

	default:									/*NOT SUPPORT!!!*/
		return -1;
	}
}