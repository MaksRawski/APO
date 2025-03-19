#pragma once
#include <QImage>

#include <vector>

using LUT = std::vector<int>;

const int LMIN = 0;
const int LMAX = 256;
const int M = LMAX - LMIN;

// LUT histogram(const QImage &image);
LUT negate();
LUT stretch(int p1, int p2, int q3, int q4);
LUT posterize(int n);
