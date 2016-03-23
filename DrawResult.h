#pragma once

#include "RealsenseCam.h"

class DrawResult : public RealsenseCam
{
public:
	DrawResult();
	~DrawResult();

	void drawLandmark(cv::Mat &image, std::vector<LandmarkData> points);

	void drawEyeTracking(cv::Mat &image, std::vector<LandmarkData> points);

	void drawEmotion(cv::Mat &image, EmotionName emotion, EmotionName sentiment, cv::Rect rect);

	void drawExpression(cv::Mat &image, ExpressionData expressionData, cv::Rect rect);

	void drawRecognition(cv::Mat &image, RecognitionData recognitionData, cv::Rect rect);

protected:

	std::map <RealsenseCam::ExpressionName, std::string> expressionMap;
	std::map <RealsenseCam::ExpressionName, std::string> initExpressionMap();
	std::string emotionConvertEnumToString(EmotionName emotion);

};

