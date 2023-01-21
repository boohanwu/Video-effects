#include "opencv2/opencv.hpp"
#include <iostream>
#include <time.h>

using namespace cv; // 宣告 opencv 函式庫的命名空間
using namespace std; // 宣告 C++函式庫的命名空間

/* 自訂函式 */
void detectAndDisplay(void); // 偵測人臉的函式宣告

/* 全域變數 */
String face_cascade_name = "../data/haarcascade_frontalface_alt.xml";	// 正面人臉哈爾階層式分類器的參數 (PS:需要修改此檔案路徑)
String eyes_cascade_name = "../data/haarcascade_eye_tree_eyeglasses.xml";	// 人眼哈爾階層式分類器的參數 (PS:需要修改此檔案路徑)

CascadeClassifier face_cascade; // 建立正面人臉哈爾階層式分類器物件
CascadeClassifier eyes_cascade; // 建立人眼哈爾階層式分類器物件

/* 滑鼠座標 */
int mouse_x = 0;
int mouse_y = 0;

Mat im; // 輸入影像
Mat im_explosion;	// 輸入爆炸影像

/* 影像的長寬 */
double cap1_height;
double cap1_width;
double cap2_height;
double cap2_width;

int option = -1;	// 特效選項
int nowFrame = 0;	// 爆炸影片的當前幀數

VideoCapture cap1("../data/sleepy.mp4"); // 建立讀人臉視訊物件 (PS:需要修改此檔案路徑)
//VideoCapture cap1(0);	// 視訊鏡頭
VideoCapture cap2("../data/explosion.mp4"); // 建立爆破視訊物件 (PS:需要修改此檔案路徑)
Point eye_centers[2]; // 雙眼中心的(x,y)位置


//定義滑鼠反應函式
static void mouse_callback(int event, int x, int y, int flags, void*)
{
	switch (event) 
	{
		case EVENT_LBUTTONDOWN:
			break;
		case EVENT_LBUTTONUP:	// 當滑鼠按開左鍵，根據點選位置，得到選項 (option) 數值 
			if (x >= 50 && x <= 150 && y >= 380 && y <= 430)
				option = 1;		// 綠臉
			else if (x >= 200 && x <= 300 && y >= 380 && y <= 430)
				option = 2;		// 負片
			else if (x >= 350 && x <= 450 && y >= 380 && y <= 430)
				option = 3;		// 馬賽克
			else if (x >= 500 && x <= 600 && y >= 380 && y <= 430)
				option = 4;		// 殭屍臉
			else
			{
				option = 5;		// 雷射眼
				mouse_x = x;	// 回傳滑鼠座標
				mouse_y = y;
				nowFrame = 0;	// 歸零爆炸視訊的幀數
			}
			break;
	}
}

/** 主程式 */
int main(void)
{
	if (!cap1.isOpened()) { printf("--(!)Error loading viedo/camera\n"); waitKey(0); return -1; };	// 讀不到人臉視訊的處理

	// 讀取兩個視訊物件的高寬
	cap1_height = cap1.get(CAP_PROP_FRAME_HEIGHT);
	cap1_width = cap1.get(CAP_PROP_FRAME_WIDTH);
	cap2_height = cap2.get(CAP_PROP_FRAME_HEIGHT);
	cap2_width = cap2.get(CAP_PROP_FRAME_WIDTH);

	// 載入人臉與人眼分類器的參數
	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading face cascade\n"); waitKey(0); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)) { printf("--(!)Error loading eyes cascade\n"); waitKey(0); return -1; };

	while (char(waitKey(1)) != 27 && cap1.isOpened()) // 當鍵盤沒按 Esc，以及視訊物件成功開啟時，持續執行 while 迴圈
	{
		cap1 >> im; // 抓取視訊的畫面
		if (im.empty()) // 如果沒抓到畫面
		{
			printf(" --(!) No captured im -- Break!");  //顯示錯誤訊息
			break;
		}
		if (option == 5)	// 雷射眼選項
		{
			cap2 >> im_explosion;	// 啟動爆炸視訊
			if (im_explosion.empty())	// 如果沒抓到畫面
				option = -1;	// 初始化特效選項
		}
		//定義視窗名稱 namedWindow
		namedWindow("window");

		//設定滑鼠反應函式 setMouseCallback
		setMouseCallback("window", mouse_callback);

		/*偵測人臉，並顯示特效結果*/
		detectAndDisplay();
	}
	return 0;
}

/** detectAndDisplay 函式內容*/
void detectAndDisplay(void)
{
	/**人臉偵測前處理*/
	vector<Rect> faces; // 人臉ROI區域表列
	Mat im_gray; // 灰階影像物件
	Mat faceROI; // 人臉的ROI

	cvtColor(im, im_gray, COLOR_BGR2GRAY);	// 彩色影像轉灰階
	equalizeHist(im_gray, im_gray);		// 灰階值方圖等化(對比自動增強)。若視訊品質好，可不用

	face_cascade.detectMultiScale(im_gray, faces, 1.1, 4, 0, Size(80, 80));		// 正面人臉哈爾階層式分類

	/** 如果有偵測到人臉，執行以下敘述*/
	if (faces.size() > 0)
	{
		for(size_t i = 0; i < faces.size(); i++)
		{
			faceROI = im_gray(faces[i]);
			Point pt1 = Point(max(0, faces[i].x - 20), max(0, faces[i].y - 20));	// 人臉候選框的座標
			Point pt2 = Point(faces[i].x + faces[i].width + 20, faces[i].y + faces[i].height + 20);
			rectangle(im, pt1, pt2, CV_RGB(255, 255, 0), 2);	// 將人臉框起來
			putText(im, "M11007308", Point(max(0, faces[i].x - 20), max(0, faces[i].y - 30)), FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 255, 0), 2);	// 放上學號

			std::vector<Rect> eyes;
			eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(20, 20));	// 在每個人臉候選框偵測眼睛

			for (size_t j = 0; j < eyes.size(); j++)
			{
				eye_centers[j] = Point(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);	// 找出眼睛中心點
				if(eyes.size()==1)	// 如果只找到一隻眼睛
				{
					int face_centerX = faces[i].x + faces[i].width / 2;		// 人臉候選框的中心x座標
					eye_centers[1] = Point(2 * face_centerX - eye_centers[0].x, eye_centers[0].y);	// 第二隻眼睛在另一隻眼睛在人臉候選框的對稱處
				}
			}
		}
		/* 顯示特效選項 */
		rectangle(im, Point(50, 380), Point(150, 430), Scalar(0, 0, 255), 5);
		putText(im, "Green", Point(60, 410), FONT_HERSHEY_PLAIN, 1.5, CV_RGB(255, 255, 0), 2);

		rectangle(im, Point(200, 380), Point(300, 430), Scalar(0, 0, 255), 5);
		putText(im, "Negative", Point(210, 410), FONT_HERSHEY_PLAIN, 1.2, CV_RGB(255, 255, 0), 2);

		rectangle(im, Point(350, 380), Point(450, 430), Scalar(0, 0, 255), 5);
		putText(im, "Masaic", Point(360, 410), FONT_HERSHEY_PLAIN, 1.5, CV_RGB(255, 255, 0), 2);

		rectangle(im, Point(500, 380), Point(600, 430), Scalar(0, 0, 255), 5);
		putText(im, "Zombie", Point(510, 410), FONT_HERSHEY_PLAIN, 1.5, CV_RGB(255, 255, 0), 2);

		Mat skinROI,im_hsv, skinMask;	//建立多個 Mat 影像與遮罩物件
		skinROI = im(Range(max(0, faces[faces.size() - 1].y - 20), faces[faces.size() - 1].y + faces[faces.size() - 1].width + 20), 
					 Range(max(0, faces[faces.size() - 1].x - 20), faces[faces.size() - 1].x + faces[faces.size() - 1].width + 20));	// 皮膚的ROI
		cvtColor(skinROI, im_hsv, COLOR_BGR2HSV);	// 轉成hsv空間
		inRange(im_hsv, Scalar(0, 30, 40), Scalar(50, 180, 250), skinMask);		// 用 inRange 篩選膚色區域，hsv的上下界大約是(0, 30, 40)與(50, 180, 250)
		// 對遮罩做形態學膨脹運算
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3)); // 建立圓形結構元素
		dilate(skinMask, skinMask, element); // 將膚色遮罩影像膨脹
		erode(skinMask, skinMask, element); // 將膚色遮罩影像腐蝕
		
		// 選項1：史瑞克
		if (option == 1) {
			Mat im_face;
			vector<Mat> hsv_plane;
			cvtColor(skinROI, im_face, COLOR_BGR2HSV);	//將im_face 轉換到 HSV 空間(COLOR_BGR2HSV)
			split(im_face, hsv_plane);	// 將hsv向量分離出來
			hsv_plane[0] = hsv_plane[0] + 50;	// 將hue通道數據平移50
			merge(hsv_plane, im_face);	// 將hsv向量合併回去
			cvtColor(im_face, im_face, COLOR_HSV2BGR); // 轉換回BGR空間
			im_face.copyTo(im(Range(max(0, faces[faces.size() - 1].y - 20), faces[faces.size() - 1].y + faces[faces.size() - 1].width + 20),
							  Range(max(0, faces[faces.size() - 1].x - 20),	faces[faces.size() - 1].x + faces[faces.size() - 1].width + 20)));	// 將調整後的 im_hsv 數據，複製到 im_face 的膚色遮罩區域
		}
		
		// 選項2：負片
		else if (option == 2) {
			Mat im_neg = skinROI;
			for (int i = 0; i < im_neg.rows; i++)
			{
				for (int j = 0; j < im_neg.cols; j++)
				{
					im_neg.at<Vec3b>(i, j) = Vec3b(255, 255, 255) - im_neg.at<Vec3b>(i, j);		// 負片:255-每個像素的值
				}
			}
			im_neg.copyTo(im(Range(max(0, faces[faces.size() - 1].y - 20), faces[faces.size() - 1].y + faces[faces.size() - 1].width + 20),
							 Range(max(0, faces[faces.size() - 1].x - 20), faces[faces.size() - 1].x + faces[faces.size() - 1].width + 20)));	// 將調整後的負片影像，複製到 faceROI 區域
		}

		// 選項3：馬賽克
		else if (option == 3) {
			Mat im_masaic;
			resize(skinROI, im_masaic, Size(20, 20));	// 縮小人臉
			resize(im_masaic, im_masaic, skinROI.size());	// 放大人臉回原尺寸
			im_masaic.copyTo(im(Range(max(0, faces[faces.size() - 1].y - 20), faces[faces.size() - 1].y + faces[faces.size() - 1].width + 20),
								Range(max(0, faces[faces.size() - 1].x - 20), faces[faces.size() - 1].x + faces[faces.size() - 1].width + 20)));	// 將調整後的馬賽克影像，複製到 faceROI 區域
		}
		// 選項4：殭屍(臉變蒼白)
		else if (option == 4) {
			Mat im_face;
			vector<Mat> hsv_plane;
			cvtColor(skinROI, im_face, COLOR_BGR2HSV);	// 將im_face 轉換到 HSV 空間(COLOR_BGR2HSV)
			split(im_face, hsv_plane);	// 將hsv向量分離出來
			hsv_plane[1] = hsv_plane[1] - 75;	// 將saturation通道數據平移75
			merge(hsv_plane, im_face);	// 將hsv向量合併回去
			cvtColor(im_face, im_face, COLOR_HSV2BGR); // 轉換回BGR空間
			im_face.copyTo(im(Range(max(0, faces[faces.size() - 1].y - 20), faces[faces.size() - 1].y + faces[faces.size() - 1].width + 20),
							  Range(max(0, faces[faces.size() - 1].x - 20), faces[faces.size() - 1].x + faces[faces.size() - 1].width + 20)));	// 將調整後的蒼白臉影像，複製到 faceROI 區域
		}

		// 其餘選項：雷射眼
		else if (option == 5)
		{
			if (nowFrame < cap2.get(CAP_PROP_FRAME_COUNT))	// 如果爆破視訊已啟動，且當前影格小於影片幀數，則執行下列迴敘述
			{
				cap2.set(CAP_PROP_POS_FRAMES, nowFrame);	// 顯示當前幀數的影像
				nowFrame++;	// 影格編號加1
				cap2 >> im_explosion;	// 載入爆破視訊影像
				resize(im_explosion, im_explosion, Size(128, 72));	// 爆破影像縮小
				threshold(im_explosion, im_explosion, 50, 255, 0);	// 使用 threshold(src, mask, 50, 255, 0); 建立遮罩
				
				// 防呆：防止爆破視訊放在以x,y為中心的位置時，超過 im 的邊界，限制爆炸影像只能在以下區域顯示
				bool overX = im_explosion.cols < mouse_x && mouse_x < cap1_width - im_explosion.cols;	// 檢測滑鼠座標是否位在爆炸影像能正常顯示的區域
				bool overY = im_explosion.rows < mouse_y && mouse_y < cap1_height - im_explosion.rows;
				if (overX && overY)
				{
					line(im, Point(mouse_x, mouse_y), eye_centers[0], Scalar(0, 0, 255), 2);	// 根據 eye_centers 與滑鼠的x,y位置，用 line() 繪製雷射線
					line(im, Point(mouse_x, mouse_y), eye_centers[1], Scalar(0, 0, 255), 2);
					Mat explosionROI = im(Rect(mouse_x - 64, mouse_y - 36, 128, 72));	// 設定爆炸影像顯示區域
					im_explosion.copyTo(explosionROI, im_explosion);	// 根據遮罩mask，用 src.copyTo(dst, mask) 方法，將爆破影像貼入 im 的相對位置
				}
			}
			else // 否則的話(滑鼠未點擊，或爆破視訊已撥放完畢)，執行下列敘述
			{	
				nowFrame = 0;	// 將影格編號設為0
				cap2.set(CAP_PROP_POS_FRAMES, nowFrame);	// 用 cap2.set 將影格指標指到開始的位置
				cap2.release();	// 將爆破視訊啟動設為"否"
				option = -1;	// 特效影像初始化
			}
		}
	}
	moveWindow("window", 100, 100);		// 移動視窗位置
	imshow("window", im);	// 顯示影像
}

