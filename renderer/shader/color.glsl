const vec4 kRed = vec4(1, 0, 0, 1);
const vec4 kOrange = vec4(1, 156 / 255.0, 0, 1);
const vec4 kYellow = vec4(1, 1, 0, 1);
const vec4 kGreen = vec4(0, 1, 0, 1);
const vec4 kCyan = vec4(0, 1, 1, 1);
const vec4 kBlue = vec4(0, 0, 1, 1);
const vec4 kPurple = vec4(1, 0, 1, 1);
const vec4 kBlack = vec4(0, 0, 0, 1);
const vec4 kGray = vec4(0.5, 0.5, 0.5, 1);
const vec4 kWhite = vec4(1, 1, 1, 1);

const vec4 kRed2 = vec4(1 / 2., 0, 0, 1);
const vec4 kOrange2 = vec4(1 / 2., 156 / 2. / 255.0, 0, 1);
const vec4 kYellow2 = vec4(1 / 2., 1 / 2., 0, 1);
const vec4 kGreen2 = vec4(0, 1 / 2., 0, 1);
const vec4 kCyan2 = vec4(0, 1 / 2., 1 / 2., 1);
const vec4 kBlue2 = vec4(0, 0, 1 / 2., 1);
const vec4 kPurple2 = vec4(1 / 2., 0, 1 / 2., 1);
const vec4 kBlack2 = vec4(0, 0, 0, 1);
const vec4 kGray2 = vec4(0.5 / 2., 0.5 / 2., 0.5 / 2., 1);
const vec4 kWhite2 = vec4(1 / 2., 1 / 2., 1 / 2., 1);

const vec4 kRed3 = vec4(1 / 3., 0, 0, 1);
const vec4 kOrange3 = vec4(1 / 3., 156 / 3. / 255.0, 0, 1);
const vec4 kYellow3 = vec4(1 / 3., 1 / 3., 0, 1);
const vec4 kGreen3 = vec4(0, 1 / 3., 0, 1);
const vec4 kCyan3 = vec4(0, 1 / 3., 1 / 3., 1);
const vec4 kBlue3 = vec4(0, 0, 1 / 3., 1);
const vec4 kPurple3 = vec4(1 / 3., 0, 1 / 3., 1);
const vec4 kBlack3 = vec4(0, 0, 0, 1);
const vec4 kGray3 = vec4(0.5 / 3., 0.5 / 3., 0.5 / 3., 1);
const vec4 kWhite3 = vec4(1 / 3., 1 / 3., 1 / 3., 1);

const vec4 kClearColor = vec4(0.45, 0.55, 0.60, 1.00);

const int kColorCircleNum = 24;
const vec4 kColorCircle[kColorCircleNum] = vec4[kColorCircleNum] (
  kRed, kOrange, kYellow, kGreen, kCyan, kBlue, kPurple, kWhite,
  kRed2, kOrange2, kYellow2, kGreen2, kCyan2, kBlue2, kPurple2, kWhite2,
  kRed3, kOrange3, kYellow3, kGreen3, kCyan3, kBlue3, kPurple3, kWhite3
);