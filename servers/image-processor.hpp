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
    virtual ~BaseProcessor() = default;  // ✅ Required
};

// Decorators
class GrayscaleProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;

public:
    explicit GrayscaleProcessor(std::unique_ptr<ImageProcessor> processor)
        : wrapped_processor(std::move(processor)) {}

    Mat process(const Mat &image) override;
    virtual ~GrayscaleProcessor() = default;  // ✅ Required
};

class ResizeProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int width, height;

public:
    ResizeProcessor(std::unique_ptr<ImageProcessor> processor, int w, int h)
        : wrapped_processor(std::move(processor)), width(w), height(h) {}

    Mat process(const Mat &image) override;
    virtual ~ResizeProcessor() = default;  // ✅ Required
};

class BlurProcessor : public ImageProcessor
{
private:
    std::unique_ptr<ImageProcessor> wrapped_processor;
    int kernel_size;

public:
    BlurProcessor(std::unique_ptr<ImageProcessor> processor, int kernel)
        : wrapped_processor(std::move(processor)), kernel_size(kernel) {}

    Mat process(const Mat &image) override;
    virtual ~BlurProcessor() = default;  // ✅ Required
};