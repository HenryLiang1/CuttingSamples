#include <opencv2\objdetect\objdetect.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\core\core.hpp>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

IplImage* Imagex;																//Original
IplImage* Image;																//Modified
CvPoint VertexOne, VertexThree;													//長方形的左上點和右下點
int key;																		//按鍵碼
IplImage* frame;
Mat dest;
bool isMBClicked = false;
vector<CvRect> positiveROI;
vector<CvRect> negativeROI;
int Mode = 0;
Size standardSize = Size(12, 12);												// 標準大小
bool sizeChackMode = true;														// 是否要有設限大小功能
bool scaleSmoothMode = true;													// 是否要有滑鼠拖曳功能
double scale = 1;																// 當前縮放比例
double minimumScale = 0.5;														// 最小縮放比例
double maximalScale = 30;														// 最大縮放比例
Size minimumSize = Size(standardSize.width * minimumScale, standardSize.height * minimumScale);		// 限定最小範圍
Size maximalSize = Size(standardSize.width * maximalScale, standardSize.height * maximalScale);		// 限定最大範圍
double scaleInterval = 0.25;													// 按鈕改變縮放間隔
double scaleSmoothInterval = 0.05;												// 滑鼠拖曳縮放間隔
Point2f mousePoi;																// 滑鼠位置
bool isSmoothing;
enum SCALE_MODE { SCALE_UP, SCALE_DOWN };
enum SCALE_DIR {RightUp, RightDown, LeftDown, LeftUp};
SCALE_DIR currentScaleDir;
Size WindowSize;
bool isRatioScale = false;
CvPoint current;

void refresh(Point2f poi, CvPoint current);
void ChangeSize(SCALE_MODE mode);
bool inRange(Size currentSize);
void smoothScaleCheck(CvPoint src, CvPoint dest, double &scaleDest, enum SCALE_DIR &scaleDir);

void ChangeSize(SCALE_MODE mode) 
{
	double afterScale = 0;
	Size afterSize;
	bool isDefaultAct = false;
	switch (mode) {
	case SCALE_UP:
		afterScale = (scale + scaleInterval);
		if (afterScale > maximalScale) {
			afterScale = maximalScale;
		}
		afterSize = Size(afterScale * standardSize.width, afterScale * standardSize.height);
		break;
	case SCALE_DOWN:
		afterScale = (scale - scaleInterval);
		if (afterScale < minimumScale) {
			afterScale = minimumScale;
		}
		afterSize = Size(afterScale * standardSize.width, afterScale * standardSize.height);
		break;
	default:
		cout << "default action!" << endl;
		isDefaultAct = true;
		break;
	}
	if (!isDefaultAct && inRange(afterSize))
	{
		scale = afterScale;
		if (isMBClicked) {
			refresh(mousePoi, current);
		}
	}
}

bool inRange(Size currentSize) 
{
	if (sizeChackMode && (currentSize.width >= minimumSize.width && currentSize.height >= minimumSize.height) && (currentSize.width <= maximalSize.width && currentSize.height <= maximalSize.height)) {
		return true;
	}
	return false;
}

void onMouse(int Event, int x, int y, int flags, void* param)
{
	//得到左上角座標
	if (Event == CV_EVENT_LBUTTONDOWN) 
	{
		// remember mouse point
		mousePoi.x = x;
		mousePoi.y = y;

		current.x = x;
		current.y = y;
		refresh(mousePoi, current);
		isSmoothing = false;
		currentScaleDir = RightUp;
	}
	if (Event == CV_EVENT_LBUTTONUP)
	{
		isSmoothing = false;
	}
	// 拖曳滑鼠中
	if (flags == CV_EVENT_FLAG_LBUTTON)
	{
		double standard = sqrt(pow(minimumSize.width, 2) + pow(minimumSize.height, 2));
		double dis = sqrt(pow((x - mousePoi.x), 2) + pow((y - mousePoi.y), 2));
		// 拖曳模式剛開始
		if (dis >= standard) {
			isSmoothing = true;
		}
		if (isSmoothing) 
		{
			double currentScale = 0;
			current.x = x;
			current.y = y;
			if (isRatioScale) {
				smoothScaleCheck(mousePoi, current, currentScale, currentScaleDir);
				scale = currentScale;
			}
			
			refresh(mousePoi, current);
		}
	}
}

void smoothScaleCheck(CvPoint src, CvPoint dest, double &scaleDest, enum SCALE_DIR &scaleDir) 
{
	double standard = sqrt(pow(standardSize.width, 2) + pow(standardSize.height, 2));
	double dis = sqrt(pow((dest.x - src.x), 2) + pow((dest.y - src.y), 2)) / standard;
	double quantize = (int)(dis / scaleSmoothInterval) * scaleSmoothInterval;
	if (quantize < minimumScale)
		quantize = minimumScale;
	if (quantize > maximalScale)
		quantize = maximalScale;
	scaleDest = quantize;

	if (dest.x >= src.x && dest.y >= src.y)
		scaleDir = RightUp;
	if (dest.x >= src.x && dest.y <= src.y)
		scaleDir = RightDown;
	if (dest.x < src.x && dest.y > src.y)
		scaleDir = LeftUp;
	if (dest.x < src.x && dest.y < src.y)
		scaleDir = LeftDown;
}

void refresh(Point2f poi, CvPoint current) {
	VertexOne = cvPoint(poi.x, poi.y);
	Size currentSize = Size(scale * standardSize.width, scale * standardSize.height);
	double xDir = 1, yDir = 1;
	switch (currentScaleDir)
	{
	case RightUp:
		xDir = 1;
		yDir = 1;
		break;
	case RightDown:
		xDir = 1;
		yDir = -1;
		break;
	case LeftUp:
		xDir = -1;
		yDir = 1;
		break;
	case LeftDown:
		xDir = -1;
		yDir = -1;
		break;
	}

	Mat img1 = dest.clone();
	if (isRatioScale) {
		VertexThree = Point(VertexOne.x + currentSize.width * xDir, VertexOne.y + currentSize.height * yDir);		
		rectangle(img1, Point(VertexOne.x, VertexOne.y), Point(VertexThree.x, VertexThree.y), Scalar(10, 10, 255), 1, CV_AA, 0);
	}
	else {
		VertexThree = Point(current.x * xDir, current.y * yDir);
		rectangle(img1, Point(VertexOne.x, VertexOne.y), Point(VertexThree.x, VertexThree.y), Scalar(10, 10, 255), 1, CV_AA, 0);
	}
	//Mat img1 = dest.clone();
	//rectangle(img1, Point(VertexOne.x, VertexOne.y), Point(VertexThree.x, VertexThree.y), Scalar(10, 10, 255), 1, CV_AA, 0);
	isMBClicked = true;
	imshow("frame", img1);
}

int main(int argc, char** argv)
{
	int posFileCount = 0, negFileCount = 0;//計數
	VideoCapture capture("MIO_train//neg/FILE150103-220316.MP4");//影片路徑
	cout << "請確認大寫功能已關閉" << endl;
	if (!capture.isOpened())
	{
		cout << "fail to open!" << endl;
		system("pause");
		return 0;
	}
	cvNamedWindow("frame", WINDOW_AUTOSIZE);
	cvSetMouseCallback("frame", onMouse, NULL);
	long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "Total Frames: " << totalFrameNumber << endl;
	long frameToStart = 0;
	capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	cout << "start from " << frameToStart << " frames" << endl;
	long currentFrame = frameToStart;
	bool stop = false;
	Mat frame;
	bool isNext = true;
	bool isChangeSize = false;
	while (!stop)
	{
		if (currentFrame > totalFrameNumber) {
			break;
		}
		if (isNext)
		{
			capture.read(frame);
			resize(frame, frame, Size(640, 480));
		}
		if (frame.empty())
		{
			break;
		}
		if (!isChangeSize) {
			cout << "now frame: " << currentFrame << " now mode: " << Mode << endl;
			dest = frame.clone();
			imshow("frame", dest);
			isChangeSize = false;
		}

		int c = waitKey();
		if ((char)c == 27 || currentFrame > totalFrameNumber) //esc
		{
			stop = true;
			isNext = false;
		}
		else if (c == 114) //save positive(r)
		{
			if (isMBClicked)
			{
				CvRect tempRect = cvRect(VertexOne.x, VertexOne.y, abs(VertexOne.x - VertexThree.x), abs(VertexOne.y - VertexThree.y));
				if (VertexThree.x < frame.cols && VertexThree.y < frame.rows && VertexOne.x >= 0 && VertexOne.y >= 0)
				{
					positiveROI.push_back(tempRect);
				}
				isNext = false;
				isChangeSize = false;
			}
		}
		else if (c == 121) //save negative(y)
		{
			if (isMBClicked)
			{
				CvRect tempRect = cvRect(VertexOne.x, VertexOne.y, abs(VertexOne.x - VertexThree.x), abs(VertexOne.y - VertexThree.y));
				if (VertexThree.x < frame.cols && VertexThree.y < frame.rows && VertexOne.x >= 0 && VertexOne.y >= 0)
				{
					negativeROI.push_back(tempRect);
				}
				isNext = false;
				isChangeSize = false;
			}
		}
		else if (c == 119 || c == 101) 
		{
			/**
				Key W: scale up
				Key E: scale down
			*/
			c == 119 ? ChangeSize(SCALE_DOWN) : ChangeSize(SCALE_UP);
			isNext = false;
			isChangeSize = true;
		}
		else if (c == 32) // space
		{
			Mat ROIDest;
			for (int index = 0; index < positiveROI.size(); index++)
			{
				Mat ROI = dest(positiveROI[index]);
				//cv::resize(ROI, ROIDest, standardSize);
				Mat ROIDest;
				if (isRatioScale) {
					cv::resize(ROI, ROIDest, standardSize);
				}
				else {
					ROIDest = ROI.clone();
				}
				
				string s;
				stringstream ss(s);
				ss << posFileCount;
				imwrite("pos\\" + ss.str() + ".jpg", ROIDest);
				posFileCount++;
			}
			for (int index = 0; index < negativeROI.size(); index++)
			{
				Mat ROI = dest(negativeROI[index]);
				if (isRatioScale) {
					cv::resize(ROI, ROIDest, standardSize);
				}
				else {
					ROIDest = ROI.clone();
				}
				
				string s;
				stringstream ss(s);
				ss << negFileCount;
				imwrite("neg\\" + ss.str() + ".jpg", ROIDest);
				negFileCount++;
			}
			isMBClicked = false;
			positiveROI.clear();
			negativeROI.clear();
			currentFrame++;
			isNext = true;
			isChangeSize = false;
		}
		else if (c == 8 || c == 2162688) { // backspace || PageUp
			currentFrame -= 5;
			if (currentFrame <= 0)
				currentFrame = 0;
			positiveROI.clear();
			negativeROI.clear();
			capture.set(CV_CAP_PROP_POS_FRAMES, currentFrame);
			isChangeSize = false;
			isNext = true;
		}
		else if (c == 2228224) { // PageDown
			currentFrame += 5;
			positiveROI.clear();
			negativeROI.clear();
			capture.set(CV_CAP_PROP_POS_FRAMES, currentFrame);
			isChangeSize = false; 
			isNext = true;
		}
		else if (c = 115) {
			isRatioScale = !isRatioScale;
			cout << "Ratio Scale : " << isRatioScale;
		}
	}
	return 0;
}
