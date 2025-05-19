#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>

#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
using namespace cv;

class ImageProcessor
{
public:
    virtual Mat process(const Mat &image) = 0;
    virtual ~ImageProcessor() = default;
};

// Concrete component: Base Processor (no processing)
class BaseProcessor : public ImageProcessor
{
public:
    Mat process(const Mat &image) override;
    virtual ~BaseProcessor() = default;
};

// Decorators
class GrayscaleProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    explicit GrayscaleProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~GrayscaleProcessor() = default;
};

class ResizeProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int width, height;

public:
    ResizeProcessor(std::unique_ptr<ImageProcessor> processor, int w, int h);

    Mat process(const Mat &image) override;
    virtual ~ResizeProcessor() = default;
};

class BlurProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int kernel_size;

public:
    BlurProcessor(std::unique_ptr<ImageProcessor> processor, int kernel);

    Mat process(const Mat &image) override;
    virtual ~BlurProcessor() = default;
};

class EdgeDetectionProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    EdgeDetectionProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~EdgeDetectionProcessor() = default;
};

class RotateProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    double angle;

public:
    RotateProcessor(std::unique_ptr<ImageProcessor> processor, double angle);

    Mat process(const Mat &image) override;
    virtual ~RotateProcessor() = default;
};

class BrightnessContrastProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int brightness;
    double contrast;

public:
    BrightnessContrastProcessor(std::unique_ptr<ImageProcessor> processor, int b, double c);

    Mat process(const Mat &image) override;
    virtual ~BrightnessContrastProcessor() = default;
};

class SharpenProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    SharpenProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~SharpenProcessor() = default;
};

class EqualizeHistogramProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    EqualizeHistogramProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~EqualizeHistogramProcessor() = default;
};

class GammaCorrectionProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    double gamma;

public:
    GammaCorrectionProcessor(std::unique_ptr<ImageProcessor> processor, double g);

    Mat process(const Mat &image) override;
    virtual ~GammaCorrectionProcessor() = default;
};

class WatermarkProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    std::string text;

public:
    WatermarkProcessor(std::unique_ptr<ImageProcessor> processor, const std::string &text);

    Mat process(const Mat &image) override;
    virtual ~WatermarkProcessor() = default;
};

class ColorInversionProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    ColorInversionProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~ColorInversionProcessor() = default;
};

class SepiaProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    SepiaProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~SepiaProcessor() = default;
};

class MedianBlurProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int kernel_size;  // ← make sure this line exists!

public:
    MedianBlurProcessor(std::unique_ptr<ImageProcessor> processor, int kernel_size);
    Mat process(const Mat &image) override;
    virtual ~MedianBlurProcessor() = default;
};


class HistogramStretchProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    HistogramStretchProcessor(std::unique_ptr<ImageProcessor> processor);

    Mat process(const Mat &image) override;
    virtual ~HistogramStretchProcessor() = default;
};

class UnsharpMaskProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    double strength;

public:
    UnsharpMaskProcessor(std::unique_ptr<ImageProcessor> processor, double s);

    Mat process(const Mat &image) override;
    virtual ~UnsharpMaskProcessor() = default;
};

class DilationProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int kernel_size; // ← Add this line

public:
    DilationProcessor(std::unique_ptr<ImageProcessor> processor, int k);
    cv::Mat process(const cv::Mat &image) override;
    virtual ~DilationProcessor() = default;
};

class ErosionProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int kernel_size; // ← Add this line

public:
    ErosionProcessor(std::unique_ptr<ImageProcessor> processor, int k);
    cv::Mat process(const cv::Mat &image) override;
    virtual ~ErosionProcessor() = default;
};

class CLAHEProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    double clip_limit;

public:
    CLAHEProcessor(std::unique_ptr<ImageProcessor> processor, double clip_limit);

    Mat process(const Mat &image) override;
    virtual ~CLAHEProcessor() = default;
};
