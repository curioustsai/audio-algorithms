#include "ui_object.h"
#include "detect_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

using namespace ::testing;
using namespace ubnt::smartcv;

TEST(detect_utils, sensitivity_confidence_conversion) {
    ASSERT_FLOAT_EQ(sensitivityToConfidence(50, UIObjectType::PERSON,
                                            {{UIObjectType::PERSON, UIObjectConf(0.8, 0.5)}}),
                    0.8f);
    ASSERT_FLOAT_EQ(sensitivityToConfidence(0, UIObjectType::PERSON,
                                            {{UIObjectType::PERSON, UIObjectConf(0.8, 0.5)}}),
                    1.f);
    ASSERT_FLOAT_EQ(sensitivityToConfidence(100, UIObjectType::PERSON,
                                            {{UIObjectType::PERSON, UIObjectConf(0.8, 0.5)}}),
                    0.5f);
}

TEST(detect_utils, NMS) {
    UIObjects testObjects{
        UIObject(cv::Point2f(0, 0), cv::Point2f(100, 100), UIObjectType::PERSON, 0.f, 1),
        UIObject(cv::Point2f(0, 0), cv::Point2f(110, 110), UIObjectType::PERSON, 0.5f, 2),
        UIObject(cv::Point2f(0, 0), cv::Point2f(120, 120), UIObjectType::PERSON, 0.8f, 3),
        UIObject(cv::Point2f(100, 100), cv::Point2f(200, 200), UIObjectType::PERSON, 0.6f, 4),
        UIObject(cv::Point2f(100, 100), cv::Point2f(220, 220), UIObjectType::PERSON, 0.7f, 5)};
    UIObjects output;
    HardSingleTypeNMS(testObjects, output);
    ASSERT_EQ(output.size(), 2);
    ASSERT_FLOAT_EQ(output.front().confidence, 0.8);
    ASSERT_FLOAT_EQ(output.back().confidence, 0.7);
}
