#include "stdafx.h"
#include "DrawResult.h"

#include <string>
#include <strstream>

DrawResult::DrawResult()
{
	expressionMap = initExpressionMap();
}


DrawResult::~DrawResult()
{
}

void DrawResult::drawLandmark(cv::Mat &image, std::vector<LandmarkData> points)
{
	std::strstream ss;
	std::string strPointIdx;
	for (int i = 0; i < points.size(); ++i)
	{
		int x = (int)points[i].imagePoint.x + LANDMARK_ALIGNMENT;
		int y = (int)points[i].imagePoint.y + LANDMARK_ALIGNMENT;
		//std::cout << iter->confidenceImage << " ";
		ss << i;
		ss >> strPointIdx;
		ss.clear();
		if (points[i].confidenceImage)
			//cv::putText(image, strPointIdx, cv::Point(x, y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255));
			cv::circle(image, cv::Point(x, y), 1, cv::Scalar(255, 255, 255));
		else
			//cv::putText(image, strPointIdx, cv::Point(x, y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(0, 0, 255));
			cv::circle(image, cv::Point(x, y), 1, cv::Scalar(0, 0, 255));
	}
	//std::cout << std::endl;
}

void DrawResult::drawEyeTracking(cv::Mat &image, std::vector<LandmarkData> points)
{
	std::strstream ss;
	std::string strPointIdx;
	for (int i = 0; i < points.size(); ++i)
	{
		if ((i >= 10 && i <= 25)/*circle*/ || i == 76/*right*/ || i == 77/*left*/)
		{
			int x = (int)points[i].imagePoint.x + LANDMARK_ALIGNMENT;
			int y = (int)points[i].imagePoint.y + LANDMARK_ALIGNMENT;
			//std::cout << iter->confidenceImage << " ";
			ss << i;
			ss >> strPointIdx;
			ss.clear();
			if (points[i].confidenceImage)
				//cv::putText(image, strPointIdx, cv::Point(x, y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255));
				cv::circle(image, cv::Point(x, y), 1, cv::Scalar(0, 0, 255));
			else
				//cv::putText(image, strPointIdx, cv::Point(x, y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(0, 0, 255));
				cv::circle(image, cv::Point(x, y), 1, cv::Scalar(0, 0, 255));
		}
	}
	//std::cout << std::endl;
}

void DrawResult::drawEmotion(cv::Mat &image, EmotionName emotion, EmotionName sentiment, cv::Rect rect)
{
	std::string strEmotion = emotionConvertEnumToString(emotion);
	std::string strSentiment = emotionConvertEnumToString(sentiment);
	cv::putText(image, strEmotion, cv::Point(rect.x, rect.y + rect.height + 20),
		cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 255));
	//cv::putText(image, strSentiment, cv::Point(rect.x, rect.y + rect.height+ 30),
	//	cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255));
}

void DrawResult::drawExpression(cv::Mat &image, ExpressionData expressionData, cv::Rect rect)
{
	std::strstream ss;
	int space = 15;
	int count = 1;
	for (auto iter = expressionMap.begin(); iter != expressionMap.end(); ++iter)
	{
		if (iter->first == RealsenseCam::ExpressionName::EXPRESSION_EYES_CLOSED_LEFT ||
			iter->first == RealsenseCam::ExpressionName::EXPRESSION_EYES_CLOSED_RIGHT ||
			iter->first == RealsenseCam::ExpressionName::EXPRESSION_EYES_TURN_LEFT ||
			iter->first == RealsenseCam::ExpressionName::EXPRESSION_EYES_TURN_RIGHT ||
			iter->first == RealsenseCam::ExpressionName::EXPRESSION_EYES_UP ||
			iter->first == RealsenseCam::ExpressionName::EXPRESSION_EYES_DOWN)
		{
			//show the intensity of every expression
			//std::string strIntersity;
			//ss << expressionData.intensity[iter->first];
			//ss >> strIntersity; ss.clear();
			//std::string strExpression = iter->second + " : " + strIntersity;
			//cv::putText(image, strExpression, cv::Point(rect.x + rect.width, rect.y + count*space),
			//	cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
			//++count;

			//only show the expression result
			if (expressionData.intensity[iter->first] == 100)
			{
				cv::putText(image, iter->second, cv::Point(rect.x + rect.width, rect.y + count*space),
					cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 0, 255));
				++count;
			}
		}
	}
}

void DrawResult::drawRecognition(cv::Mat &image, RecognitionData recognitionData, cv::Rect rect)
{
	std::string strRecognition;
	if (recognitionData.isRegistered)
	{
		std::strstream ss;
		std::string strID;
		ss << recognitionData.UserID;
		ss >> strID; ss.clear();
		strRecognition = "UserID: " + strID;
	}
	else
	{
		strRecognition = "User is not registered!";
	}
	cv::putText(image, strRecognition, cv::Point(rect.x, rect.y),
		cv::FONT_HERSHEY_PLAIN, 1.3, cv::Scalar(0, 0, 255));
}


std::string DrawResult::emotionConvertEnumToString(EmotionName emotion)
{
	switch (emotion)
	{
	case RealsenseCam::EMOTION_NULL:
		return "NULL";
	case RealsenseCam::EMOTION_PRIMARY_ANGER:
		return "ANGER";
	case RealsenseCam::EMOTION_PRIMARY_CONTEMPT:
		return "CONTEMPT";
	case RealsenseCam::EMOTION_PRIMARY_DISGUST:
		return "DISGUST";
	case RealsenseCam::EMOTION_PRIMARY_FEAR:
		return "FEAR";
	case RealsenseCam::EMOTION_PRIMARY_JOY:
		return "JOY";
	case RealsenseCam::EMOTION_PRIMARY_SADNESS:
		return "SANDESS";
	case RealsenseCam::EMOTION_PRIMARY_SURPRISE:
		return "SURPRISE";
	case RealsenseCam::EMOTION_SENTIMENT_POSITIVE:
		return "POSITIVE";
	case RealsenseCam::EMOTION_SENTIMENT_NEGATIVE:
		return "NEGATIVE";
	case RealsenseCam::EMOTION_SENTIMENT_NEUTRAL:
		return "NEUTRAL";
	default:
		return "NULL";
	}
}

std::map <RealsenseCam::ExpressionName, std::string> DrawResult::initExpressionMap()
{
	std::map <RealsenseCam::ExpressionName, std::string> map;
	map[RealsenseCam::ExpressionName::EXPRESSION_BROW_RAISER_LEFT] = std::string("Brow Raised Left");
	map[RealsenseCam::ExpressionName::EXPRESSION_BROW_RAISER_RIGHT] = std::string("Brow Raised Right");
	map[RealsenseCam::ExpressionName::EXPRESSION_BROW_LOWERER_LEFT] = std::string("Brow Lowered Left");
	map[RealsenseCam::ExpressionName::EXPRESSION_BROW_LOWERER_RIGHT] = std::string("Brow Lowered Right");

	map[RealsenseCam::ExpressionName::EXPRESSION_SMILE] = std::string("Smile");
	map[RealsenseCam::ExpressionName::EXPRESSION_KISS] = std::string("Kiss");
	map[RealsenseCam::ExpressionName::EXPRESSION_MOUTH_OPEN] = std::string("Mouth Open");

	map[RealsenseCam::ExpressionName::EXPRESSION_EYES_CLOSED_LEFT] = std::string("Closed Eye Left");
	map[RealsenseCam::ExpressionName::EXPRESSION_EYES_CLOSED_RIGHT] = std::string("Closed Eye Right");

	map[RealsenseCam::ExpressionName::EXPRESSION_HEAD_TURN_LEFT] = std::string("Head Turn Left");
	map[RealsenseCam::ExpressionName::EXPRESSION_HEAD_TURN_RIGHT] = std::string("Head Turn Right");
	map[RealsenseCam::ExpressionName::EXPRESSION_HEAD_UP] = std::string("Head Up");
	map[RealsenseCam::ExpressionName::EXPRESSION_HEAD_DOWN] = std::string("Head Down");
	map[RealsenseCam::ExpressionName::EXPRESSION_HEAD_TILT_LEFT] = std::string("Head Tilt Left");
	map[RealsenseCam::ExpressionName::EXPRESSION_HEAD_TILT_RIGHT] = std::string("Head Tilt Right");


	map[RealsenseCam::ExpressionName::EXPRESSION_EYES_TURN_LEFT] = std::string("Eyes Turn Left");
	map[RealsenseCam::ExpressionName::EXPRESSION_EYES_TURN_RIGHT] = std::string("Eyes Turn Right");
	map[RealsenseCam::ExpressionName::EXPRESSION_EYES_UP] = std::string("Eyes Up");
	map[RealsenseCam::ExpressionName::EXPRESSION_EYES_DOWN] = std::string("Eyes Down");
	map[RealsenseCam::ExpressionName::EXPRESSION_TONGUE_OUT] = std::string("Tongue Out");
	map[RealsenseCam::ExpressionName::EXPRESSION_PUFF_RIGHT] = std::string("Puff Right Cheek");
	map[RealsenseCam::ExpressionName::EXPRESSION_PUFF_LEFT] = std::string("Puff Left Cheek");
	return map;
}