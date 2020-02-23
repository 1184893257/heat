#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cassert>
#include "ocr.h"

using namespace std;
using namespace cv;

extern bool ocr_debug;
#define DEBUG if (ocr_debug)

typedef struct
{
	Size2i dilateSize;
} OCR_OPTION;

class OCRBuilderImpl : public OCRBuilder
{
public:
	~OCRBuilderImpl() {}
	
    OCRBuilder* setPath(const string& path)
	{
		this->path = path;
		return this;
	}
    OCRBuilder* setGrayRange(int min, int max)
	{
		this->min = min;
		this->max = max;
		return this;
	}
    string ocr();

	string path;
	int min = 225;
	int max = 250;
	OCR_OPTION currentOption;
	Mat debugMat;

private:
	vector<OCR_OPTION> options
	{
		{
			{3, 10}
		},
		{
			{1, 10}
		},
		{
			{3, 6}// 宽一点避免5被上下分隔
		},
		{
			{1, 6}// 窄一点避免粘连上冒号
		}
	};
};


class OCRImpl
{
public:
	OCRImpl(OCRBuilderImpl* builder): params(builder)
	{
	}
	~OCRImpl() {}

	void cutNums(const Mat& input, vector<Mat>& output);

	string ocr();
private:
	OCRBuilderImpl* params;
};


// 无缺失旋转，可输入任何图片
void rotate(Mat& input, float avAng)
{
	// 获取二维旋转的仿射变换矩阵
	int height = input.rows;
	int width = input.cols;

	Point2f center;
	center.x = float(width / 2.0);
	center.y = float(height / 2.0);
	auto m = getRotationMatrix2D(center, avAng, 1);
	//建立输出图像RotateRow
	/**/double a = sin(avAng / 180 * CV_PI);
	double b = cos(avAng / 180 * CV_PI);
	int width_rotate = int(height*fabs(a) + width*fabs(b));
	int height_rotate = int(width*fabs(a) + height*fabs(b));
	assert(m.type() == CV_64FC1);	// 应该是 double 类型
	*((double*)m.ptr(0, 2)) += (width_rotate - width) / 2;
	*((double*)m.ptr(1, 2)) += (height_rotate - height) / 2;

	Mat output;
	auto outSize = Size(width_rotate, height_rotate);
	warpAffine(input, output, m, outSize, INTER_LINEAR + WARP_FILL_OUTLIERS);
	input = output;
}

void rotate(const string& path, float rot, const vector<int>& clip)
{
	Mat image = imread(path, cv::IMREAD_UNCHANGED);
	rotate(image, rot);

	Rect clipRect(Point(0, 0), image.size());
	if (clip.size() == 4)
	{
		clipRect = Rect(clip[0], clip[1], image.cols - clip[2], image.rows - clip[3]);
	}

	imwrite(path, image(clipRect));
}

// 自动做倾斜校正, 倾斜角度必须在30度以内
void autoRotate(Mat& input, Mat& origin)
{
	// 边缘检测减少计算量，实验后发现不能进行边缘检测，会缺少信息，算不准旋转角度
	//Mat borders;
	//Canny(input, borders, 30, 200, 3);
	//DEBUG imshow("Canny", borders);

	vector<Vec2f> lines;
	HoughLines(input, lines, 1, CV_PI / 180, 30);

	DEBUG {
		float alpha = 1000;
		for (size_t i = 0; i < lines.size(); i++)
		{
			float rho = lines[i][0], theta = lines[i][1];
			double cs = cos(theta), sn = sin(theta);
			double x = rho * cs, y = rho * sn;
			Point pt1(cvRound(x + alpha * (-sn)), cvRound(y + alpha * cs));
			Point pt2(cvRound(x - alpha * (-sn)), cvRound(y - alpha * cs));
			line(origin, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
		}
		//line(origin, Point(0, 0), Point(400, 400), Scalar(0, 0, 255), 1, LINE_AA);
		imshow("originCopy", origin);
	}

	int numLine = 0;
	double sumAng = 0.0;
	for (size_t i = 0; i < lines.size(); ++i)
	{
		float theta = lines[i][1];
		if (theta < 30 * CV_PI / 180)
		{
			numLine++;
			sumAng = sumAng + theta;
		}
		else if ((CV_PI - theta) < 30 * CV_PI / 180)
		{
			numLine++;
			sumAng = sumAng + theta - CV_PI;
		}
	}
	//计算出平均倾斜角，anAng为角度制
	float avAng = (float)((sumAng / numLine) * 180 / CV_PI);

	rotate(input, avAng);

	DEBUG imshow("rotate", input);
}

void OCRImpl::cutNums(const Mat& input, vector<Mat>& output)
{
	vector<vector<Point> > contours_out;
	vector<Vec4i> hierarchy;
	findContours(input, contours_out, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	// re-arrange location according to the real position in the original image 
	const size_t size = contours_out.size();
	vector<Rect> num_location;
	int maxHeight = 0;
	for (int i = 0; i < contours_out.size(); i++)
	{
		Rect bound = boundingRect(Mat(contours_out[i]));
		num_location.push_back(bound);// 转换为矩形轮廓
		if (bound.height > maxHeight)
		{
			maxHeight = bound.height;
		}
	}

	{
		Mat dest;
		cvtColor(input, dest, COLOR_GRAY2RGB);
		for (auto& rect : num_location)
		{
			rectangle(dest, rect, Scalar(0, 0, 255), 1);
		}
		DEBUG imshow("rects", dest);

		if (params->debugMat.cols == 0)
		{
			params->debugMat = dest;
		}
		else
		{
			Mat tmp;
			hconcat(params->debugMat, dest, tmp);
			params->debugMat = tmp;
		}
	}

	// 过滤高度不对的
	for (auto it = num_location.begin(); it != num_location.end();)
	{
		if ((*it).height < 0.7 * maxHeight)
		{
			it = num_location.erase(it);
		}
		else
		{
			it++;
		}
	}

	sort(num_location.begin(), num_location.end(), [](const Rect& a, const Rect& b)
	{
		if (a.x < b.x)
			return true;
		else
			return false;
	}); // 重排轮廓信息

	int tube_num = 0;
	for (auto& rect : num_location)
	{
		output.push_back(input(rect));

		/*char rectnum[10];
		_itoa_s(tube_num, rectnum, 10);
		imshow(string(rectnum), output.at(tube_num));
		tube_num++;*/
	}
}

bool Iswhite(const Mat& input, int top, int bottom, int left, int right)
{
	for (int i = top; i <= bottom; ++i)
	{
		for (int j = left; j <= right; ++j)
		{
			auto p = input.ptr(i, j);
			if (255 == *p)
				return true;
		}
	}
	return false;
}

int TubeIdentification(Mat inputmat) // 穿线法判断数码管a、b、c、d、e、f、g、
{
	int tube = 0;
	int tubo_roi[7][4] =
	{
		{ inputmat.rows * 0 / 3, inputmat.rows * 1 / 3, inputmat.cols * 1 / 2, inputmat.cols * 1 / 2 }, // a
		{ inputmat.rows * 1 / 3, inputmat.rows * 1 / 3, inputmat.cols * 2 / 3, inputmat.cols - 1 }, // b
		{ inputmat.rows * 2 / 3, inputmat.rows * 2 / 3, inputmat.cols * 2 / 3, inputmat.cols - 1 }, // c
		{ inputmat.rows * 2 / 3, inputmat.rows - 1    , inputmat.cols * 1 / 2, inputmat.cols * 1 / 2 }, // d
		{ inputmat.rows * 2 / 3, inputmat.rows * 2 / 3, inputmat.cols * 0 / 3, inputmat.cols * 1 / 3 }, // e
		{ inputmat.rows * 1 / 3, inputmat.rows * 1 / 3, inputmat.cols * 0 / 3, inputmat.cols * 1 / 3 }, // f
		{ inputmat.rows * 1 / 3, inputmat.rows * 2 / 3, inputmat.cols * 1 / 2, inputmat.cols * 1 / 2 }, // g
	};

	if (inputmat.rows / inputmat.cols > 2)   // 1 is special, which is much narrower than others
	{
		tube = 6;
	}
	else
	{
		for (int i = 0; i < 7; i++)
		{
			if (Iswhite(inputmat, tubo_roi[i][0], tubo_roi[i][1], tubo_roi[i][2], tubo_roi[i][3]))
				tube = tube + (1 << i);
		}
	}

	switch (tube)
	{
	case  63: return 0;  break;
	case   6: return 1;  break;
	case  91: return 2;  break;
	case  79: return 3;  break;
	case 102: return 4;  break;
	case 109: return 5;  break;
	case 125: return 6;  break;
	case   7: return 7;  break;
	case 127: return 8;  break;
	case 111: return 9;  break;

	default: return -1;
	}
}

string OCRImpl::ocr()
{
	cv::Mat image = cv::imread(params->path, cv::IMREAD_GRAYSCALE);

	Mat image_resized = image;
	// resize(image, image_resized, Size(image.cols / 6, image.rows / 6));

	Mat originCopy;
	DEBUG cvtColor(image_resized, originCopy, COLOR_GRAY2RGB);

 	// convert to binary image
	Mat& image_bin0 = image;
	threshold(image_resized, image_bin0, params->max, 255, THRESH_TOZERO_INV);
	Mat& image_bin = image_resized;
	threshold(image_bin0, image_bin, params->min, 255, THRESH_BINARY);
	DEBUG imshow("image_bin", image_bin);

	Mat& image_ero = image_bin0;
	Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
	erode(image_bin, image_ero, element);
	DEBUG imshow("image_ero", image_ero);

	// 垂直矫正
	autoRotate(image_ero, originCopy);

	Mat& image_dil = image_bin;
	element = getStructuringElement(MORPH_RECT, params->currentOption.dilateSize);
	dilate(image_ero, image_dil, element);
	DEBUG imshow("image_dil", image_dil);

	// 切割
	vector<Mat> nums;
	cutNums(image_dil, nums);

	// 穿线法识别数字
	string result;
	for (auto& mat : nums)
	{
		int value = TubeIdentification(mat);
		if (value < 0)
		{
			DEBUG imshow("error num", mat);
		}
		else
		{
			result += '0' + value;
		}
	}

	DEBUG waitKey(0);

	return result;
}

string OCRBuilderImpl::ocr()
{
	string ret;
	int missed = 0;
	debugMat = Mat();
	for (auto& option : options)
	{
		this->currentOption = option;
		ret = OCRImpl(this).ocr();
		if (ret.length() == 3)
		{
			if (missed)
			{
				auto tmp = option;
				option = options[0];
				options[0] = tmp;
			}
			break;
		}
		missed++;
	}
	imwrite(path + ".debug.png", debugMat);
	DEBUG imshow("debugMat", debugMat);
	DEBUG waitKey(0);
	debugMat = Mat();
	return ret;
}

unique_ptr<OCRBuilder> createOCRBuilder()
{
	return make_unique<OCRBuilderImpl>();
}