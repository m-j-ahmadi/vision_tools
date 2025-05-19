#include "image-processor.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo.hpp>

// BaseProcessor
Mat BaseProcessor::process(const Mat &image) {
    return image.clone();
}

// Grayscale
GrayscaleProcessor::GrayscaleProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

Mat GrayscaleProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat gray;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    cvtColor(gray, gray, COLOR_GRAY2BGR);
    return gray;
}

// Resize
ResizeProcessor::ResizeProcessor(std::unique_ptr<ImageProcessor> processor, int w, int h)
    : wrapped_processor(std::move(processor)), width(w), height(h) {}

Mat ResizeProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat resized;
    resize(input, resized, Size(width, height));
    return resized;
}

// Blur
BlurProcessor::BlurProcessor(std::unique_ptr<ImageProcessor> processor, int kernel)
    : wrapped_processor(std::move(processor)), kernel_size(kernel) {}

Mat BlurProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat blurred;
    GaussianBlur(input, blurred, Size(kernel_size, kernel_size), 0);
    return blurred;
}

// Edge Detection
EdgeDetectionProcessor::EdgeDetectionProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

Mat EdgeDetectionProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat gray, edges;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    Canny(gray, edges, 100, 200);
    cvtColor(edges, edges, COLOR_GRAY2BGR);
    return edges;
}

// Rotate
RotateProcessor::RotateProcessor(std::unique_ptr<ImageProcessor> processor, double angle)
    : wrapped_processor(std::move(processor)), angle(angle) {}

Mat RotateProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Point2f center(input.cols / 2.0F, input.rows / 2.0F);
    Mat rot = getRotationMatrix2D(center, angle, 1.0);
    Mat rotated;
    warpAffine(input, rotated, rot, input.size());
    return rotated;
}

// Brightness and Contrast
BrightnessContrastProcessor::BrightnessContrastProcessor(std::unique_ptr<ImageProcessor> processor, int b, double c)
    : wrapped_processor(std::move(processor)), brightness(b), contrast(c) {}

Mat BrightnessContrastProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat adjusted;
    input.convertTo(adjusted, -1, contrast, brightness);
    return adjusted;
}

// Sharpen
SharpenProcessor::SharpenProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

Mat SharpenProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat kernel = (Mat_<float>(3,3) <<
                  0, -1, 0,
                 -1, 5,-1,
                  0, -1, 0);
    Mat sharpened;
    filter2D(input, sharpened, input.depth(), kernel);
    return sharpened;
}


// Gamma Correction
GammaCorrectionProcessor::GammaCorrectionProcessor(std::unique_ptr<ImageProcessor> processor, double g)
    : wrapped_processor(std::move(processor)), gamma(g) {}

Mat GammaCorrectionProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat lut(1, 256, CV_8UC1);
    for (int i = 0; i < 256; ++i)
        lut.at<uchar>(i) = pow(i / 255.0, gamma) * 255.0;
    Mat result;
    LUT(input, lut, result);
    return result;
}

// Watermark
WatermarkProcessor::WatermarkProcessor(std::unique_ptr<ImageProcessor> processor, const std::string &txt)
    : wrapped_processor(std::move(processor)), text(txt) {}

Mat WatermarkProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    putText(input, text, Point(10, input.rows - 10), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    return input;
}


ColorInversionProcessor::ColorInversionProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

cv::Mat ColorInversionProcessor::process(const cv::Mat& image) {
    cv::Mat input = wrapped_processor->process(image);
    cv::Mat inverted;
    cv::bitwise_not(input, inverted);
    return inverted;
}


// Sepia
SepiaProcessor::SepiaProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

Mat SepiaProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat kernel = (Mat_<float>(3,3) <<
         0.272, 0.534, 0.131,
         0.349, 0.686, 0.168,
         0.393, 0.769, 0.189);
    Mat sepia;
    transform(input, sepia, kernel);
    return sepia;
}

// Median Blur
MedianBlurProcessor::MedianBlurProcessor(std::unique_ptr<ImageProcessor> processor, int k)
    : wrapped_processor(std::move(processor)), kernel_size(k) {}

Mat MedianBlurProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat blurred;
    medianBlur(input, blurred, kernel_size);
    return blurred;
}

// Histogram Stretch
HistogramStretchProcessor::HistogramStretchProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

Mat HistogramStretchProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat result;
    normalize(input, result, 0, 255, NORM_MINMAX);
    return result;
}

// Unsharp Mask
UnsharpMaskProcessor::UnsharpMaskProcessor(std::unique_ptr<ImageProcessor> processor, double s)
    : wrapped_processor(std::move(processor)), strength(s) {}

Mat UnsharpMaskProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat blurred;
    GaussianBlur(input, blurred, Size(0, 0), 3);
    Mat sharp = input + strength * (input - blurred);
    return sharp;
}

// Dilation
DilationProcessor::DilationProcessor(std::unique_ptr<ImageProcessor> processor, int k)
    : wrapped_processor(std::move(processor)), kernel_size(k) {}

Mat DilationProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat result;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));
    dilate(input, result, kernel);
    return result;
}

// Erosion
ErosionProcessor::ErosionProcessor(std::unique_ptr<ImageProcessor> processor, int k)
    : wrapped_processor(std::move(processor)), kernel_size(k) {}

Mat ErosionProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat result;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(kernel_size, kernel_size));
    erode(input, result, kernel);
    return result;
}

// CLAHE
CLAHEProcessor::CLAHEProcessor(std::unique_ptr<ImageProcessor> processor, double clip)
    : wrapped_processor(std::move(processor)), clip_limit(clip) {}

Mat CLAHEProcessor::process(const Mat &image) {
    Mat input = wrapped_processor->process(image);
    Mat lab_image;
    cvtColor(input, lab_image, COLOR_BGR2Lab);
    std::vector<Mat> lab_planes(3);
    split(lab_image, lab_planes);
    Ptr<CLAHE> clahe = createCLAHE(clip_limit);
    clahe->apply(lab_planes[0], lab_planes[0]);
    merge(lab_planes, lab_image);
    Mat result;
    cvtColor(lab_image, result, COLOR_Lab2BGR);
    return result;
}

// EqualizeHistogramProcessor
EqualizeHistogramProcessor::EqualizeHistogramProcessor(std::unique_ptr<ImageProcessor> processor)
    : wrapped_processor(std::move(processor)) {}

cv::Mat EqualizeHistogramProcessor::process(const cv::Mat& image) {
    cv::Mat input = wrapped_processor->process(image);
    cv::Mat ycrcb;
    cv::cvtColor(input, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    cv::equalizeHist(channels[0], channels[0]);
    cv::merge(channels, ycrcb);
    cv::Mat result;
    cv::cvtColor(ycrcb, result, cv::COLOR_YCrCb2BGR);
    return result;
}
