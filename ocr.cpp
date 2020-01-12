#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

void rotate(Mat& input)
{
	// 边缘检测减少计算量
	Mat borders;
	Canny(input, borders, 30, 200, 3);
	imshow("Canny", borders);

	vector<Vec2f> lines;
	HoughLines(borders, lines, 1, CV_PI / 180, 20);

	int numLine = 0;
	float sumAng = 0.0;
	for (size_t i = 0; i < lines.size(); ++i)
	{
		float theta = lines[i][1];
		if (theta<30 * CV_PI / 180 || (CV_PI - theta)<30 * CV_PI / 180)
		{
			numLine++;
			sumAng = sumAng + theta;
		}
	}
	//计算出平均倾斜角，anAng为角度制
	float avAng = (sumAng / numLine) * 180 / CV_PI;

	// 获取二维旋转的仿射变换矩阵
	int height = input.rows;
	int width = input.cols;

	Point2f center;
	center.x = float(width / 2.0);
	center.y = float(height / 2.0);
	auto m = getRotationMatrix2D_(center, avAng, 1);
	//建立输出图像RotateRow
	double a = sin(avAng / 180 * CV_PI);
	double b = cos(avAng / 180 * CV_PI);
	int width_rotate = int(height*fabs(a) + width*fabs(b));
	int height_rotate = int(width*fabs(a) + height*fabs(b));
	m(0, 2) += (width_rotate - width) / 2;
	m(1, 2) += (height_rotate - height) / 2;

	Mat& output = borders;
	auto outSize = Size(width_rotate, height_rotate);
	warpAffine(input, output, m, outSize, INTER_LINEAR + WARP_FILL_OUTLIERS);

	input = output;
	imshow("rotate", input);
}

// 测试无缺失旋转，可输入任何图片
void testRotate(Mat& input)
{
	//计算出平均倾斜角，anAng为角度制
	float avAng = 30;

	// 获取二维旋转的仿射变换矩阵
	int height = input.rows;
	int width = input.cols;

	Point2f center;
	center.x = float(width / 2.0);
	center.y = float(height / 2.0);
	auto m = getRotationMatrix2D_(center, avAng, 1);
	//建立输出图像RotateRow
	/**/double a = sin(avAng / 180 * CV_PI);
	double b = cos(avAng / 180 * CV_PI);
	int width_rotate = int(height*fabs(a) + width*fabs(b));
	int height_rotate = int(width*fabs(a) + height*fabs(b));
	m(0, 2) += (width_rotate - width) / 2;
	m(1, 2) += (height_rotate - height) / 2;

	Mat output;
	auto outSize = Size(width_rotate, height_rotate);
	warpAffine(input, output, m, outSize, INTER_LINEAR + WARP_FILL_OUTLIERS);

	imshow("justRotate", output);
}

string ocr(const string& picturePath)
{
	cv::Mat image = cv::imread(picturePath, cv::IMREAD_GRAYSCALE);

	Mat image_gry;
	resize(image, image_gry, Size(image.cols / 6, image.rows / 6));

	//testRotate(image_gry);

	Mat& image_bin = image;
	threshold(image_gry, image_bin, 240, 255, THRESH_BINARY); // convert to binary image
	//imshow("image_bin", image_bin);

	Mat& image_ero = image_gry;
	Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
	erode(image_bin, image_ero, element);
	//imshow("image_ero", image_ero);

	Mat& image_dil = image_bin;
	element = getStructuringElement(MORPH_RECT, Size(6, 6));
	dilate(image_ero, image_dil, element);
	imshow("image_dil", image_dil);

	// 垂直矫正
	rotate(image_dil);

	vector<vector<Point> > contours_out;
	vector<Vec4i> hierarchy;
	findContours(image_dil, contours_out, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	// re-arrange location according to the real position in the original image 
	const size_t size = contours_out.size();
	vector<Rect> num_location;
	for (int i = 0; i < contours_out.size(); i++)
	{
		num_location.push_back(boundingRect(Mat(contours_out[i])));// 转换为矩形轮廓
	}
	sort(num_location.begin(), num_location.end(), [](const Rect& a, const Rect& b)
	{
		if (a.x < b.x)
			return true;
		else
			return false;
	}); // 重排轮廓信息

	cv::waitKey(0);

	return "";
}