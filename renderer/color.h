#pragma once

#include "glm/glm.hpp"

namespace renderer {
const glm::vec4 kRed = glm::vec4(1, 0, 0, 1);
const glm::vec4 kRed2 = glm::vec4(1 / 2., 0, 0, 1);
const glm::vec4 kRed3 = glm::vec4(1 / 3., 0, 0, 1);
const glm::vec4 kOrange = glm::vec4(1, 156 / 255.0, 0, 1);
const glm::vec4 kOrange2 = glm::vec4(1 / 2., 156 / 2. / 255.0, 0, 1);
const glm::vec4 kOrange3 = glm::vec4(1 / 3., 156 / 2. / 255.0, 0, 1);
const glm::vec4 kYellow = glm::vec4(1, 1, 0, 1);
const glm::vec4 kYellow2 = glm::vec4(1 / 2., 1 / 2., 0, 1);
const glm::vec4 kYellow3 = glm::vec4(1 / 3., 1 / 3., 0, 1);
const glm::vec4 kGreen = glm::vec4(0, 1, 0, 1);
const glm::vec4 kGreen2 = glm::vec4(0, 1 / 2., 0, 1);
const glm::vec4 kGreen3 = glm::vec4(0, 1 / 3., 0, 1);
const glm::vec4 kCyan = glm::vec4(0, 1, 1, 1);
const glm::vec4 kCyan2 = glm::vec4(0, 1 / 2., 1 / 2., 1);
const glm::vec4 kCyan3 = glm::vec4(0, 1 / 2., 1 / 3., 1);
const glm::vec4 kBlue = glm::vec4(0, 0, 1, 1);
const glm::vec4 kBlue2 = glm::vec4(0, 0, 1 / 2., 1);
const glm::vec4 kBlue3 = glm::vec4(0, 0, 1 / 3., 1);
const glm::vec4 kPurple = glm::vec4(1, 0, 1, 1);
const glm::vec4 kPurple2 = glm::vec4(1 / 2., 0, 1 / 2., 1);
const glm::vec4 kPurple3 = glm::vec4(1 / 3., 0, 1 / 3., 1);
const glm::vec4 kBlack = glm::vec4(0, 0, 0, 1);
const glm::vec4 kWhite = glm::vec4(1, 1, 1, 1);
const glm::vec4 kWhite2 = glm::vec4(1 / 2., 1 / 2., 1 / 2., 1);
const glm::vec4 kWhite3 = glm::vec4(1 / 3., 1 / 3., 1 / 3., 1);
const glm::vec4 kGray = glm::vec4(0.5, 0.5, 0.5, 1);

const glm::vec4 kClearColor = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

const int kColorCircleNum = 24;
const glm::vec4 kColorCircle[kColorCircleNum] = {
    kRed, kOrange, kYellow, kGreen, kCyan, kBlue, kPurple, kWhite,
    kRed2, kOrange2, kYellow2, kGreen2, kCyan2, kBlue2, kPurple2, kWhite2,
    kRed3, kOrange3, kYellow3, kGreen3, kCyan3, kBlue3, kPurple3, kWhite3};
}