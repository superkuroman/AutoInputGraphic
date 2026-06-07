#pragma once

void YYYY()
{
    // モニターサイズ取得
    //HWND desktop = GetDesktopWindow();
    //RECT rect;
    //GetWindowRect(desktop, &rect);
    //
    //
    //// DIBの情報を設定する
    //BITMAPINFO bmpInfo;
    //bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    //bmpInfo.bmiHeader.biWidth = rect.right;
    //bmpInfo.bmiHeader.biHeight = -rect.bottom;
    //bmpInfo.bmiHeader.biPlanes = 1;
    //bmpInfo.bmiHeader.biBitCount = 24; //cv::Matの画像をアルファチャンネル有りにする場合はは32;
    //bmpInfo.bmiHeader.biCompression = BI_RGB;
    //
    //
    //LPDWORD lpPixel;
    //HDC hDC = GetDC(desktop);
    //HBITMAP hBitmap = CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, (void**)&lpPixel, NULL, 0);
    //HDC hMemDC = CreateCompatibleDC(hDC);
    //SelectObject(hMemDC, hBitmap);
   // cv::Mat m1(3, 3, CV_64FC(10));
    //cv::Mat monitor_img;
    //monitor_img.create(rect.bottom, rect.right, CV_8UC3); //RGBのみ。アルファちゃんねるを加えるにはCV_8UN4

}

//void similarity()
//{
//    // 画像を読み込む
//    cv::Mat source1;
//    cv::imread("A.jpg", cv::IMREAD_COLOR).copyTo(source1);
//    if (source1.empty())
//    {
//        //throw cv::runtime_error("Failed to open image");
//    }
//    cv::Mat source2;
//    cv::imread("B.jpg", cv::IMREAD_COLOR).copyTo(source2);
//    if (source2.empty())
//    {
//       // throw cv::runtime_error("Failed to open image");
//    }
//
//    // 画像のヒストグラムを計算する
//    int imageCount = 1; // 入力画像の枚数
//    int channelsToUse[] = { 0 }; // 0番目のチャネルを使う
//    int dimention = 1; // ヒストグラムの次元数
//    int binCount = 256; // ヒストグラムのビンの数
//    int binCounts[] = { binCount };
//    float range[] = { 0, 256 }; // データの範囲は0～255
//    const float* histRange[] = { range };
//    cv::Mat histogram1;
//    calcHist(&source1, imageCount, channelsToUse, cv::Mat(), histogram1, dimention, binCounts, histRange);
//    cv::Mat histogram2;
//    calcHist(&source2, imageCount, channelsToUse, cv::Mat(), histogram2, dimention, binCounts, histRange);
//
//    // 類似度を調べる（同じ画像を読み込んだため1が出力される）CV_COMP_CORREL
//    double correlation = cv::compareHist(histogram1, histogram2,cv::HISTCMP_CORREL);
//}