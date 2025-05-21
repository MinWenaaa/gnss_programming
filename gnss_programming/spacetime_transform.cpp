#include "pch.h" 
#include <cmath>
#include <iomanip>
#include <sstream>

#include "spacetime_transform.h"

const double M_PI = 3.1415926;
void timeTransformer::run() {
    switch (origin) {
    case UTM: 
		jd = UTM2JD();
		break;
	case JD:
		// JD不需要转换
		break;
	case GPS:
		jd = GPS2JD();
		break;
	case DOY:
		jd = DOY2JD();
		break;
    default:
        break;
    }

    switch (target) {
	case UTM:
		JD2UTM();
		break;
	case JD:
		// JD不需要转换
		break;
	case GPS:
		JD2GPS();
        break;
	case DOY:
		JD2DOY();
		break;
	default:
		break;
    }
}

std::string timeTransformer::getTarget() {
    const auto& inst = timeTransformer::instance();
    std::ostringstream oss;
    switch (inst.target) {
    case timeTransformer::UTM:
        // 格式: YYYY-MM-DD HH:MM:SS
        oss << std::setfill('0')
            << std::setw(4) << inst.utm_year << '-'
            << std::setw(2) << inst.utm_mon << '-'
            << std::setw(2) << inst.utm_day << ' '
            << std::setw(2) << inst.utm_hour << ':'
            << std::setw(2) << inst.utm_min << ':'
            << std::setw(2) << inst.utm_sec;
        break;
    case timeTransformer::JD:
        // 格式: JD=xxxx.xxxxx
        oss << "JD=" << std::fixed << std::setprecision(5) << inst.jd;
        break;
    case timeTransformer::GPS:
        // 格式: GPS Week: xxxx, Seconds: xxxxx
        oss << "GPS Week: " << inst.gps_week << ", Seconds: " << inst.gps_second;
        break;
    case timeTransformer::DOY:
        // 格式: YYYY-DOY
        oss << std::setfill('0')
            << std::setw(4) << inst.doy_year << '-'
            << std::setw(3) << inst.doy_day;
        break;
    default:
        oss << "Unknown target type";
        break;
    }
    return oss.str();
}

void timeTransformer::parserInput(std::string input) {
	std::istringstream iss(input);
	if (origin == UTM) {
		// 解析UTM时间
		char delim;
		iss >> utm_year >> delim >> utm_mon >> delim >> utm_day
			>> utm_hour >> delim >> utm_min >> delim >> utm_sec;
	}
	else if (origin == JD) {
		// 解析JD时间
		iss >> jd;
	}
	else if (origin == GPS) {
		// 解析GPS时间
		iss >> gps_week >> gps_second;
	}
	else if (origin == DOY) {
		// 解析DOY时间
		char delim;
		iss >> doy_year >> delim >> doy_day;
	}
}


double timeTransformer::UTM2JD() {
    int Y = utm_year;
    int M = utm_mon;
    double D = utm_day + (utm_hour + (utm_min + utm_sec / 60.0) / 60.0) / 24.0;
    if (M <= 2) {
        Y -= 1;
        M += 12;
    }
    int A = Y / 100;
    int B = 2 - A + A / 4;

    double utc_jd = std::floor(365.25 * (Y + 4716))
        + std::floor(30.6001 * (M + 1))
        + D + B - 1524.5;

    // 查询跳秒
    int leap = getLeapSeconds(utm_year, utm_mon, utm_day);

    jd = utc_jd + leap / 86400.0;
    return jd;
}

double timeTransformer::GPS2JD() {
    // GPS起点JD为2444244.5（1980-01-06 00:00:00 UTC）
    constexpr double GPS_EPOCH_JD = 2444244.5;
    jd = GPS_EPOCH_JD + gps_week * 7 + gps_second / 86400.0;
    return jd;
}

double timeTransformer::DOY2JD() {
    // 先计算该年1月1日0时的JD
    int Y = doy_year;
    int M = 1;
    double D = 1.0; // 1月1日
    int A = Y / 100;
    int B = 2 - A + A / 4;
    double jd0 = std::floor(365.25 * (Y + 4716))
        + std::floor(30.6001 * (M + 1))
        + D + B - 1524.5;
    jd = jd0 + (doy_day - 1);
    return jd;
}

void timeTransformer::JD2UTM() {
    // 1. 估算UTC JD（先不减跳秒）
    double jd_utc = jd;

    // 2. 反复迭代，直到跳秒数收敛
    int leap = 0, leap_last = -1;
    int year, month, day;
    while (leap != leap_last) {
        // 先用当前jd_utc转年月日
        double Z = std::floor(jd_utc + 0.5);
        double F = (jd_utc + 0.5) - Z;
        double A = Z;
        if (Z >= 2299161) {
            int alpha = static_cast<int>((Z - 1867216.25) / 36524.25);
            A = Z + 1 + alpha - alpha / 4;
        }
        double B = A + 1524;
        int C = static_cast<int>((B - 122.1) / 365.25);
        int D = static_cast<int>(365.25 * C);
        int E = static_cast<int>((B - D) / 30.6001);

        double dayf = B - D - std::floor(30.6001 * E) + F;
        day = static_cast<int>(dayf);
        double day_fraction = dayf - day;

        if (E < 14)
            month = E - 1;
        else
            month = E - 13;

        if (month > 2)
            year = C - 4716;
        else
            year = C - 4715;

        leap_last = leap;
        leap = getLeapSeconds(year, month, day);
        jd_utc = jd - leap / 86400.0;
    }

    // 3. 用最终jd_utc转UTC年月日时分秒
    double Z = std::floor(jd_utc + 0.5);
    double F = (jd_utc + 0.5) - Z;
    double A = Z;
    if (Z >= 2299161) {
        int alpha = static_cast<int>((Z - 1867216.25) / 36524.25);
        A = Z + 1 + alpha - alpha / 4;
    }
    double B = A + 1524;
    int C = static_cast<int>((B - 122.1) / 365.25);
    int D = static_cast<int>(365.25 * C);
    int E = static_cast<int>((B - D) / 30.6001);

    double dayf = B - D - std::floor(30.6001 * E) + F;
    utm_day = static_cast<int>(dayf);
    double day_fraction = dayf - utm_day;

    if (E < 14)
        utm_mon = E - 1;
    else
        utm_mon = E - 13;

    if (utm_mon > 2)
        utm_year = C - 4716;
    else
        utm_year = C - 4715;

    double h = day_fraction * 24.0;
    utm_hour = static_cast<int>(h);
    double m = (h - utm_hour) * 60.0;
    utm_min = static_cast<int>(m);
    utm_sec = static_cast<int>((m - utm_min) * 60.0 + 0.5);
}

void timeTransformer::JD2GPS() {
    constexpr double GPS_EPOCH_JD = 2444244.5; // 1980-01-06 00:00:00 UTC
    double days = jd - GPS_EPOCH_JD;
    gps_week = static_cast<int>(days / 7);
    gps_second = static_cast<int>((days - gps_week * 7) * 86400 + 0.5); 
}

void timeTransformer::JD2DOY() {
	// JD转DOY
    double Z = std::floor(jd + 0.5);
    double F = (jd + 0.5) - Z;
    double A = Z;
    if (Z >= 2299161) {
        int alpha = static_cast<int>((Z - 1867216.25) / 36524.25);
        A = Z + 1 + alpha - alpha / 4;
    }
    double B = A + 1524;
    int C = static_cast<int>((B - 122.1) / 365.25);
    int D = static_cast<int>(365.25 * C);
    int E = static_cast<int>((B - D) / 30.6001);

    int day = static_cast<int>(B - D - std::floor(30.6001 * E) + F);
    int month = (E < 14) ? (E - 1) : (E - 13);
    int year = (month > 2) ? (C - 4716) : (C - 4715);

    // 计算DOY
    static const int month_days[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int doy = day;
    for (int m = 1; m < month; ++m) {
        doy += month_days[m - 1];
        // 闰年2月
        if (m == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
            ++doy;
    }
    doy_year = year;
    doy_day = doy;
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

void spaceTransformer::run() {
	if (target == BLH) {
		xyz2blh();
	}
	else if (target == ENU) {
		xyz2enu();
	}
    else if (origin == ENU) {
        enu2xyz();
    }
    else if (origin == BLH) {
        blh2xyz();
    }
}

void spaceTransformer::xyz2blh() {
    double r = sqrt(x * x + y * y);
    double B = atan2(z, r * (1 - e2)); // 初始纬度
    double N, H, B0;
    do {
        B0 = B;
        N = a / sqrt(1 - e2 * sin(B0) * sin(B0));
        H = r / cos(B0) - N;
        B = atan2(z, r * (1 - e2 * N / (N + H)));
    } while (fabs(B - B0) > 1e-11);

    b = B* 180.0 /M_PI; // 弧度
    l = atan2(y, x) * 180.0 / M_PI; // 弧度
    h = H;
}

const double true_x = 2393383.1319, true_y = 5393860.9241, true_z = 2412592.1599;

void spaceTransformer::xyz2enu() {
    double ref_b, ref_l, ref_h;
    {
        double r = sqrt(true_x * true_x + true_y * true_y);
        double B = atan2(true_z, r * (1 - e2));
        double N, H, B0;
        do {
            B0 = B;
            N = a / sqrt(1 - e2 * sin(B0) * sin(B0));
            H = r / cos(B0) - N;
            B = atan2(true_z, r * (1 - e2 * N / (N + H)));
        } while (fabs(B - B0) > 1e-11);
        ref_b = B;
        ref_l = atan2(true_y, true_x);
        ref_h = H;
    }

    double dx = x - true_x;
    double dy = y - true_y;
    double dz = z - true_z;

    e = -sin(ref_l) * dx + cos(ref_l) * dy;
    n = -sin(ref_b) * cos(ref_l) * dx - sin(ref_b) * sin(ref_l) * dy + cos(ref_b) * dz;
    u = cos(ref_b) * cos(ref_l) * dx + cos(ref_b) * sin(ref_l) * dy + sin(ref_b) * dz;
}

void spaceTransformer::parserInput(const std::string& input) {
    std::istringstream iss(input);
    char delim;
    iss >> x >> delim >> y >> delim >> z;
}

std::string spaceTransformer::getTarget() {
    std::ostringstream oss;
    if (target == XYZ) {
        oss << x << ';' << y << ';' << z;
    }
    else if (target == BLH) {
        oss << b << ';' << l << ';' << h;
    }
    else if (target == ENU) {
        oss << e << ';' << n << ';' << u;
    }
    else {
        oss << "Unknown target type";
    }
    return oss.str();
}

void spaceTransformer::blh2xyz() {
    // b, l 为角度，需转为弧度
    double B = b * M_PI / 180.0;
    double L = l * M_PI / 180.0;
    double N = a / sqrt(1 - e2 * sin(B) * sin(B));
    x = (N + h) * cos(B) * cos(L);
    y = (N + h) * cos(B) * sin(L);
    z = (N * (1 - e2) + h) * sin(B);
}

// ENU -> XYZ（需参考点true_x, true_y, true_z）
void spaceTransformer::enu2xyz() {
    // 1. 参考点xyz转blh
    double ref_b, ref_l, ref_h;
    {
        double r = sqrt(true_x * true_x + true_y * true_y);
        double B = atan2(true_z, r * (1 - e2));
        double N, H, B0;
        do {
            B0 = B;
            N = a / sqrt(1 - e2 * sin(B0) * sin(B0));
            H = r / cos(B0) - N;
            B = atan2(true_z, r * (1 - e2 * N / (N + H)));
        } while (fabs(B - B0) > 1e-11);
        ref_b = B;
        ref_l = atan2(true_y, true_x);
        ref_h = H;
    }

    // 2. ENU到差分XYZ
    double dx = -sin(ref_l) * e - sin(ref_b) * cos(ref_l) * n + cos(ref_b) * cos(ref_l) * u;
    double dy = cos(ref_l) * e - sin(ref_b) * sin(ref_l) * n + cos(ref_b) * sin(ref_l) * u;
    double dz = cos(ref_b) * n + sin(ref_b) * u;

    // 3. 得到目标XYZ
    x = true_x + dx;
    y = true_y + dy;
    z = true_z + dz;
}