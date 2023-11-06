
#include "detection.h"


void resize_and_pad(const image_t& src, image_t& dst, cv::Size new_shape, int pad_color) {
    int src_width = src.cols;
    int src_height = src.rows;
    int new_width = new_shape.width;
    int new_height = new_shape.height;
    int interpolation_method;
    int pad_top, pad_bottom, pad_left, pad_right;
    double aspect_ratio = double(src_width) / src_height;
    double new_aspect_ratio = double(new_width) / new_height;
    if (src_height > new_height || src_width > new_width){
        interpolation_method = cv::INTER_AREA;
    } else {
        interpolation_method = cv::INTER_CUBIC;
    }
    if ( (new_aspect_ratio >= aspect_ratio) || ((new_aspect_ratio == 1) && (aspect_ratio <= 1)) ){
        // new_height = new_height;
        new_width = int(new_height * aspect_ratio);
        pad_left = int(double(new_shape.width - new_width) / 2);
        pad_right = int(double(new_shape.width - new_width) / 2);
        pad_top = 0;
        pad_bottom = 0;
    }
    else {
        new_height = int(new_width / aspect_ratio);
        pad_top = int(double(new_shape.height - new_height) / 2);
        pad_bottom = int(double(new_shape.height - new_width) / 2);
        pad_left = 0;
        pad_right = 0;
    }

    cv::resize(src, dst, cv::Size(new_width, new_height), 0, 0, interpolation_method);

    color_t color = cv::Scalar(pad_color, pad_color, pad_color);
    cv::copyMakeBorder(dst, dst, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT | CV_HAL_BORDER_ISOLATED, color);
}

void softmax(cv::InputArray inblob, cv::OutputArray outblob)
{
    const cv::Mat input = inblob.getMat();
    outblob.create(inblob.size(), inblob.type());

    cv::Mat exp;
    const float max = *std::max_element(input.begin<float>(), input.end<float>());
    cv::exp((input - max), exp);
    outblob.getMat() = exp / cv::sum(exp)[0];
}

void rescale_box(const bounding_box_t& src, bounding_box_t& dst, double scale){
    dst.x = (int)((double)src.x * scale);
    dst.y = (int)((double)src.y * scale);
    dst.width = (int)((double)src.width * scale);
    dst.height = (int)((double)src.height * scale);
}