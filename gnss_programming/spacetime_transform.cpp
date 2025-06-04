#include "pch.h" 
#include <cmath>
#include <iomanip>
#include <sstream>

#include "spacetime_transform.h"
const double M_PI = 3.1415926;
const double GPS_EPOCH_JD = 2444244.5;

void utc2gps(UTC* utcTime, GPS* gpsTime) {
    double D = utcTime->d + (utcTime->h + (utcTime->minu + utcTime->sec / 60.0) / 60.0) / 24.0;
    int M = utcTime->m, Y = utcTime->y;
    if (M <= 2) {
        Y -= 1; M += 12;
    }
    int A = Y / 100;
    int B = 2 - A + A / 4;

    double utc_jd = std::floor(365.25 * (Y + 4716))
        + std::floor(30.6001 * (M + 1))
        + D + B - 1524.5;

    // 查询跳秒
    int leap = getLeapSeconds(utcTime->y, utcTime->m, utcTime->d);

    double days = utc_jd - GPS_EPOCH_JD;
    gpsTime->week = static_cast<int>(days / 7);
    gpsTime->sec = static_cast<int>((days - gpsTime->week * 7) * 86400 + 0.5);

    return;
}

void gps2utc(GPS* gpsTime, UTC* utcTime) {

   double gpsSeconds = gpsTime->week * 7 * 24 * 3600 + gpsTime->sec;

   time_t utcEpoch = static_cast<time_t>(gpsSeconds + 315964800 - getLeapSeconds(1980, 1, 6));

   std::tm utcTm = {};
   if (gmtime_s(&utcTm, &utcEpoch) != 0) {
       throw std::runtime_error("无效的GPS时间");
   }

   utcTime->y = utcTm.tm_year + 1900;
   utcTime->m = utcTm.tm_mon + 1;
   utcTime->d = utcTm.tm_mday;
   utcTime->h = utcTm.tm_hour;
   utcTime->minu = utcTm.tm_min;
   utcTime->sec = utcTm.tm_sec + (gpsTime->sec - static_cast<int>(gpsTime->sec));
}

int getLeapSeconds(int year, int month, int day) {
    int leap = 0;
    for (int i = 0; i < leap_seconds_count; ++i) {
        const LeapSecond& ls = leap_seconds[i];
        if (year > ls.year ||
            (year == ls.year && month > ls.month) ||
            (year == ls.year && month == ls.month && day > ls.day)) {
            leap = ls.total_leap_seconds;
        }
        else {
            break;
        }
    }
    return leap;
}

namespace spaceTransformer {
    void xyz2blh(double x, double y, double z, double& b, double& l, double& h) {
        double r = sqrt(x * x + y * y);
        double B = atan2(z, r * (1 - e2)); // 初始纬度
        double N, H, B0;
        do {
            B0 = B;
            N = a / sqrt(1 - e2 * sin(B0) * sin(B0));
            H = r / cos(B0) - N;
            B = atan2(z, r * (1 - e2 * N / (N + H)));
        } while (fabs(B - B0) > 1e-11);

        b = B * 180.0 / M_PI; // 弧度
        l = atan2(y, x) * 180.0 / M_PI; // 弧度
        h = H;
    }
}