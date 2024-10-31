#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream> 
#include <vector>
#include <numeric>
#include <fstream>
#include <algorithm>
#include <string>
#include <windows.h>
#include <tchar.h>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"
#include "gxr_awb.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/hal/intrin.hpp"

using namespace std;

#define BinaryConfig 1

static char lightsourceConfig_bin[] = { 60, 63, 120, 109, 108, 32, 118, 101, 114, 115, 105, 111, 110, 61, 34, 49, 46, 48, 34, 32, 101, 110, 99, 111, 100, 105, 110, 103, 61, 34, 85, 84, 70, 45, 56, 34, 32, 115, 116, 97, 110, 100, 97, 108, 111, 110, 101, 61, 34, 121, 101, 115, 34, 63, 62, 10, 60, 76, 105, 103, 104, 116, 95, 83, 111, 117, 114, 99, 101, 115, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 56, 48, 48, 48, 107, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 50, 46, 52, 55, 54, 44, 49, 46, 50, 53, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 56, 48, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 68, 54, 53, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 50, 46, 50, 55, 48, 44, 49, 46, 53, 56, 57, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 54, 53, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 60, 33, 45, 45, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 99, 119, 102, 34, 62, 45, 45, 62, 10, 60, 33, 45, 45, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 56, 54, 52, 44, 50, 46, 51, 53, 50, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 45, 45, 62, 10, 60, 33, 45, 45, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 52, 49, 53, 48, 60, 47, 99, 99, 116, 62, 45, 45, 62, 10, 60, 33, 45, 45, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 45, 45, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 84, 76, 56, 52, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 55, 49, 57, 44, 50, 46, 51, 54, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 52, 49, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 65, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 50, 54, 44, 51, 46, 48, 52, 52, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 50, 56, 53, 54, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 60, 47, 76, 105, 103, 104, 116, 95, 83, 111, 117, 114, 99, 101, 115, 62, 10, 0 };

static char weightConfig_bin[] = { 60, 63, 120, 109, 108, 32, 118, 101, 114, 115, 105, 111, 110, 61, 34, 49, 46, 48, 34, 32, 101, 110, 99, 111, 100, 105, 110, 103, 61, 34, 85, 84, 70, 45, 56, 34, 32, 115, 116, 97, 110, 100, 97, 108, 111, 110, 101, 61, 34, 121, 101, 115, 34, 63, 62, 10, 60, 76, 105, 103, 104, 116, 95, 83, 111, 117, 114, 99, 101, 115, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 56, 48, 48, 48, 107, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 49, 46, 48, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 68, 54, 53, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 49, 46, 48, 50, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 84, 76, 56, 52, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 48, 46, 57, 56, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 108, 105, 103, 104, 116, 32, 105, 100, 61, 34, 65, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 49, 46, 48, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 108, 105, 103, 104, 116, 62, 10, 60, 47, 76, 105, 103, 104, 116, 95, 83, 111, 117, 114, 99, 101, 115, 62, 10, 0 };

static char ROIConfig_bin[] = { 60, 63, 120, 109, 108, 32, 118, 101, 114, 115, 105, 111, 110, 61, 34, 49, 46, 48, 34, 32, 101, 110, 99, 111, 100, 105, 110, 103, 61, 34, 85, 84, 70, 45, 56, 34, 32, 115, 116, 97, 110, 100, 97, 108, 111, 110, 101, 61, 34, 121, 101, 115, 34, 63, 62, 10, 60, 118, 101, 114, 116, 101, 120, 115, 62, 10, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 115, 95, 103, 108, 111, 98, 97, 108, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 49, 44, 49, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 49, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 54, 53, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 51, 44, 49, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 53, 48, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 51, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 51, 56, 56, 50, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 115, 95, 103, 108, 111, 98, 97, 108, 62, 10, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 115, 95, 103, 114, 97, 121, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 50, 44, 48, 46, 56, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 50, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 54, 53, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 44, 48, 46, 56, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 53, 48, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 51, 56, 56, 50, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 115, 95, 103, 114, 97, 121, 62, 10, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 115, 95, 98, 108, 117, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 50, 44, 48, 46, 56, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 50, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 54, 53, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 44, 48, 46, 56, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 53, 48, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 51, 56, 56, 50, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 115, 95, 98, 108, 117, 101, 62, 10, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 115, 95, 121, 101, 108, 108, 111, 119, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 50, 44, 48, 46, 56, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 108, 101, 102, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 48, 46, 50, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 54, 53, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 116, 111, 112, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 44, 48, 46, 56, 53, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 53, 48, 48, 48, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 118, 101, 114, 116, 101, 120, 32, 105, 100, 61, 34, 114, 105, 103, 104, 116, 32, 98, 111, 116, 116, 111, 109, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 49, 46, 50, 44, 48, 46, 49, 60, 47, 119, 104, 105, 116, 101, 80, 111, 105, 110, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 60, 99, 99, 116, 62, 51, 56, 56, 50, 60, 47, 99, 99, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 62, 10, 32, 32, 32, 32, 60, 47, 118, 101, 114, 116, 101, 120, 115, 95, 121, 101, 108, 108, 111, 119, 62, 10, 60, 47, 118, 101, 114, 116, 101, 120, 115, 62, 0 };

static char ROIOffsetConfig_bin[] = { 60, 63, 120, 109, 108, 32, 118, 101, 114, 115, 105, 111, 110, 61, 34, 49, 46, 48, 34, 32, 101, 110, 99, 111, 100, 105, 110, 103, 61, 34, 85, 84, 70, 45, 56, 34, 32, 115, 116, 97, 110, 100, 97, 108, 111, 110, 101, 61, 34, 121, 101, 115, 34, 63, 62, 10, 60, 118, 101, 114, 116, 101, 120, 115, 62, 10, 32, 32, 32, 32, 60, 103, 114, 97, 121, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 103, 95, 111, 102, 102, 115, 101, 116, 62, 48, 46, 48, 49, 60, 47, 114, 103, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 98, 103, 95, 111, 102, 102, 115, 101, 116, 62, 48, 46, 48, 49, 60, 47, 98, 103, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 95, 111, 102, 102, 115, 101, 116, 62, 48, 46, 48, 49, 60, 47, 119, 101, 105, 103, 104, 116, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 60, 47, 103, 114, 97, 121, 62, 10, 32, 32, 32, 32, 60, 98, 108, 117, 101, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 103, 95, 111, 102, 102, 115, 101, 116, 62, 48, 46, 48, 48, 53, 60, 47, 114, 103, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 98, 103, 95, 111, 102, 102, 115, 101, 116, 62, 48, 46, 48, 48, 53, 60, 47, 98, 103, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 95, 111, 102, 102, 115, 101, 116, 62, 48, 46, 48, 48, 53, 60, 47, 119, 101, 105, 103, 104, 116, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 60, 47, 98, 108, 117, 101, 62, 10, 32, 32, 32, 32, 60, 121, 101, 108, 108, 111, 119, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 114, 103, 95, 111, 102, 102, 115, 101, 116, 62, 45, 48, 46, 48, 48, 53, 60, 47, 114, 103, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 98, 103, 95, 111, 102, 102, 115, 101, 116, 62, 45, 48, 46, 48, 48, 53, 60, 47, 98, 103, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 95, 111, 102, 102, 115, 101, 116, 62, 45, 48, 46, 48, 48, 53, 60, 47, 119, 101, 105, 103, 104, 116, 95, 111, 102, 102, 115, 101, 116, 62, 10, 32, 32, 32, 32, 60, 47, 121, 101, 108, 108, 111, 119, 62, 10, 60, 47, 118, 101, 114, 116, 101, 120, 115, 62, 0 };

static char seriesConfig_bin[] = { 60, 63, 120, 109, 108, 32, 118, 101, 114, 115, 105, 111, 110, 61, 34, 49, 46, 48, 34, 32, 101, 110, 99, 111, 100, 105, 110, 103, 61, 34, 85, 84, 70, 45, 56, 34, 32, 115, 116, 97, 110, 100, 97, 108, 111, 110, 101, 61, 34, 121, 101, 115, 34, 63, 62, 10, 60, 115, 101, 114, 105, 101, 115, 62, 10, 32, 32, 32, 32, 60, 102, 114, 97, 109, 101, 32, 105, 100, 61, 34, 49, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 48, 46, 48, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 102, 114, 97, 109, 101, 62, 10, 32, 32, 32, 32, 60, 102, 114, 97, 109, 101, 32, 105, 100, 61, 34, 50, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 48, 46, 49, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 102, 114, 97, 109, 101, 62, 10, 32, 32, 32, 32, 60, 102, 114, 97, 109, 101, 32, 105, 100, 61, 34, 51, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 48, 46, 50, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 102, 114, 97, 109, 101, 62, 10, 32, 32, 32, 32, 60, 102, 114, 97, 109, 101, 32, 105, 100, 61, 34, 52, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 48, 46, 51, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 102, 114, 97, 109, 101, 62, 10, 32, 32, 32, 32, 60, 102, 114, 97, 109, 101, 32, 105, 100, 61, 34, 53, 34, 62, 10, 32, 32, 32, 32, 32, 32, 32, 32, 60, 119, 101, 105, 103, 104, 116, 62, 48, 46, 52, 60, 47, 119, 101, 105, 103, 104, 116, 62, 10, 32, 32, 32, 32, 60, 47, 102, 114, 97, 109, 101, 62, 10, 60, 47, 115, 101, 114, 105, 101, 115, 62, 10, 0 };

void AWBAlgo::readStats(int* p, int blknum_)
{
	stats = p;
	blknum = blknum_;
}

void AWBAlgo::readLightXML(const char *file_name)
{
    const int buf_len = 2048;
    char buf[buf_len] = { 0 };
    int i = 0;
    try
    {
        // 1.清空缓冲区
        memset(buf,0,buf_len);
        // 2.拼接绝对路径
        std::string strXml = "./";
        strXml.append(file_name);
        // 3.用file文件读入缓冲区
        rapidxml::file<> fdoc(strXml.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(fdoc.data());
        rapidxml::xml_node<>* root = doc.first_node();
        rapidxml::xml_node<>* node = root->first_node();
        rapidxml::xml_node<>* whitePoint = node->first_node();
        const char *d = ",";
        char *p;

        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
			whitepoints[i] = 1.0 / tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (node->next_sibling())
        {
            node = node->next_sibling();
            whitePoint = node->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
				whitepoints[i] = 1.0 / tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
    }catch(rapidxml::parse_error e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

void AWBAlgo::readSeriesXML(const char *file_name)
{
    const int buf_len = 2048;
    char buf[buf_len] = { 0 };
    int i = 0;
    try
    {
        // 1.清空缓冲区
        memset(buf,0,buf_len);
        // 2.拼接绝对路径
        std::string strXml = "./";
        strXml.append(file_name);
        // 3.用file文件读入缓冲区
        rapidxml::file<> fdoc(strXml.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(fdoc.data());
        rapidxml::xml_node<>* root = doc.first_node();
        rapidxml::xml_node<>* node = root->first_node();
        rapidxml::xml_node<>* weight = node->first_node();
        const char *d = ",";
        char *p;

        p = strtok(weight->value(),d);
        while(p)
        {
            float tmp = stof(p);
			serial_coef[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }

        while (node->next_sibling())
        {
            node = node->next_sibling();
            weight = node->first_node();
            char *q = strtok(weight->value(),d);
            while(q)
            {
                float tmp = stof(q);
				serial_coef[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
    }catch(rapidxml::parse_error e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

void AWBAlgo::readROIXML(const char *file_name)
{
    const int buf_len = 2048;
    char buf[buf_len] = { 0 };
    int i = 0;
    try
    {
        // 1.清空缓冲区
        memset(buf,0,buf_len);
        // 2.拼接绝对路径
        std::string strXml = "./";
        strXml.append(file_name);
        // 3.用file文件读入缓冲区
        rapidxml::file<> fdoc(strXml.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(fdoc.data());
        rapidxml::xml_node<>* vertexs = doc.first_node();
		// 获取全局区域
        rapidxml::xml_node<>* vertexs_global = vertexs->first_node("vertexs_global");
        rapidxml::xml_node<>* vertex = vertexs_global->first_node();
        rapidxml::xml_node<>* whitePoint = vertex->first_node();
        const char *d = ",";
        char *p;

        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
			ROIList[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
				ROIList[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
        //获取灰色有效区域
        i = 0;
        rapidxml::xml_node<>* vertexs_gray = vertexs->first_node("vertexs_gray");
        vertex = vertexs_gray->first_node();
        whitePoint = vertex->first_node();
        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
            ROIGrayList[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
				ROIGrayList[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
        //获取蓝色区域
        i = 0;
        rapidxml::xml_node<>* vertexs_blue = vertexs->first_node("vertexs_blue");
        vertex = vertexs_blue->first_node();
        whitePoint = vertex->first_node();
        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
            ROIBlueList[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
                ROIBlueList[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
        //获取黄色区域
        i = 0;
        rapidxml::xml_node<>* vertexs_yellow = vertexs->first_node("vertexs_yellow");
        vertex = vertexs_yellow->first_node();
        whitePoint = vertex->first_node();
        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
            ROIYellowList[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
                ROIYellowList[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }

    }catch(rapidxml::parse_error e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return;
}

void AWBAlgo::readROIOffsetXML(const char *file_name)
{
    const int buf_len = 2048;
    char buf[buf_len] = { 0 };
    int i = 0;
    try
    {
        // 1.清空缓冲区
        memset(buf,0,buf_len);
        // 2.拼接绝对路径
        std::string strXml = "./";
        strXml.append(file_name);
        // 3.用file文件读入缓冲区
        rapidxml::file<> fdoc(strXml.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(fdoc.data());
        rapidxml::xml_node<>* vertexs = doc.first_node();
        rapidxml::xml_node<>* vertexs_gray = vertexs->first_node("gray");
        rapidxml::xml_node<>* vertex = vertexs_gray->first_node();
        rapidxml::xml_node<>* whitePoint = vertex->first_node();
        const char *d = ",";
        char *p;

        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
            ROIGrayOffset[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
                ROIGrayOffset[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
        //获取第二个颜色区域
        i = 0;
        rapidxml::xml_node<>* vertexs_blue = vertexs->first_node("blue");
        vertex = vertexs_blue->first_node();
        whitePoint = vertex->first_node();
        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
            ROIBlueOffset[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
                ROIBlueOffset[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
        //获取第三个颜色区域
        i = 0;
        rapidxml::xml_node<>* vertexs_yellow = vertexs->first_node("yellow");
        vertex = vertexs_yellow->first_node();
        whitePoint = vertex->first_node();
        p = strtok(whitePoint->value(),d);
        while(p)
        {
            float tmp = stof(p);
            ROIYellowOffset[i] = tmp;
            i += 1;
            p=strtok(NULL,d);

        }
        while (vertex->next_sibling())
        {
            vertex = vertex->next_sibling();
            whitePoint = vertex->first_node();
            char *q = strtok(whitePoint->value(),d);
            while(q)
            {
                float tmp = stof(q);
                ROIYellowOffset[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }

    }catch(rapidxml::parse_error e)
    {
        std::cout<<e.what()<<std::endl;
    }
    return;
}

void AWBAlgo::readWeightXML(const char *file_name)
{
    const int buf_len = 2048;
    char buf[buf_len] = { 0 };
    int i = 0;
    try
    {
        memset(buf,0,buf_len);
        std::string strXml = "./";
        strXml.append(file_name);
        rapidxml::file<> fdoc(strXml.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(fdoc.data());
        rapidxml::xml_node<>* root = doc.first_node();
        rapidxml::xml_node<>* node = root->first_node();
        rapidxml::xml_node<>* weight = node->first_node();
        const char *d = ",";
        char *p;

        p = strtok(weight->value(),d);
        while(p)
        {
            float tmp = stof(p);
			weights[i] = tmp;
            i += 1;
            p=strtok(NULL,d);
        }
        while (node->next_sibling())
        {
            node = node->next_sibling();
            weight = node->first_node();
            char *q = strtok(weight->value(),d);
            while(q)
            {
                float tmp = stof(q);
				weights[i] = tmp;
                i += 1;
                q=strtok(NULL,d);
            }
        }
    }catch(rapidxml::parse_error e)
    {
        std::cout<<e.what()<<std::endl;
    }
}

void AWBAlgo::readLightXML(char data[], size_t len)
{
	const int buf_len = 2048;
	char buf[buf_len] = { 0 };
	int i = 0;
	try
	{
		memset(buf, 0, buf_len);
		rapidxml::xml_document<> doc;
		doc.parse<0>(data);
		rapidxml::xml_node<>* root = doc.first_node();
		rapidxml::xml_node<>* node = root->first_node();
		rapidxml::xml_node<>* whitePoint = node->first_node();
		const char* d = ",";
		char* p;

		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			whitepoints[i] = 1.0 / tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (node->next_sibling())
		{
			node = node->next_sibling();
			whitePoint = node->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				whitepoints[i] = 1.0 / tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
	}
	catch (rapidxml::parse_error e)
	{
		std::cout << e.what() << std::endl;
	}
}

void AWBAlgo::readSeriesXML(char data[], size_t len)
{
	const int buf_len = 2048;
	char buf[buf_len] = { 0 };
	int i = 0;
	try
	{
		memset(buf, 0, buf_len);
		rapidxml::xml_document<> doc;
		doc.parse<0>(data);
		rapidxml::xml_node<>* root = doc.first_node();
		rapidxml::xml_node<>* node = root->first_node();
		rapidxml::xml_node<>* weight = node->first_node();
		const char* d = ",";
		char* p;

		p = strtok(weight->value(), d);
		while (p)
		{
			float tmp = stof(p);
			serial_coef[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}

		while (node->next_sibling())
		{
			node = node->next_sibling();
			weight = node->first_node();
			char* q = strtok(weight->value(), d);
			while (q)
			{
				float tmp = stof(q);
				serial_coef[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
	}
	catch (rapidxml::parse_error e)
	{
		std::cout << e.what() << std::endl;
	}
}

void AWBAlgo::readROIXML(char data[], size_t len)
{
	const int buf_len = 2048;
	char buf[buf_len] = { 0 };
	int i = 0;
	try
	{
		memset(buf, 0, buf_len);
		rapidxml::xml_document<> doc;
		doc.parse<0>(data);
		rapidxml::xml_node<>* vertexs = doc.first_node();
		rapidxml::xml_node<>* vertexs_gray = vertexs->first_node("vertexs_gray");
		rapidxml::xml_node<>* vertex = vertexs_gray->first_node();
		rapidxml::xml_node<>* whitePoint = vertex->first_node();
		const char* d = ",";
		char* p;

		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIGrayList[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIGrayList[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
		//获取全局有效区域
		i = 0;
		rapidxml::xml_node<>* vertexs_global = vertexs->first_node("vertexs_global");
		vertex = vertexs_global->first_node();
		whitePoint = vertex->first_node();
		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIList[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIList[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
		//获取蓝色区域
		i = 0;
		rapidxml::xml_node<>* vertexs_blue = vertexs->first_node("vertexs_blue");
		vertex = vertexs_blue->first_node();
		whitePoint = vertex->first_node();
		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIBlueList[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIBlueList[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
		//获取黄色区域
		i = 0;
		rapidxml::xml_node<>* vertexs_yellow = vertexs->first_node("vertexs_yellow");
		vertex = vertexs_yellow->first_node();
		whitePoint = vertex->first_node();
		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIYellowList[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIYellowList[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}

	}
	catch (rapidxml::parse_error e)
	{
		std::cout << e.what() << std::endl;
	}
	return;
}

void AWBAlgo::readROIOffsetXML(char data[], size_t len)
{
	const int buf_len = 2048;
	char buf[buf_len] = { 0 };
	int i = 0;
	try
	{
		memset(buf, 0, buf_len);
		rapidxml::xml_document<> doc;
		doc.parse<0>(data);
		rapidxml::xml_node<>* vertexs = doc.first_node();
		rapidxml::xml_node<>* vertexs_gray = vertexs->first_node("gray");
		rapidxml::xml_node<>* vertex = vertexs_gray->first_node();
		rapidxml::xml_node<>* whitePoint = vertex->first_node();
		const char* d = ",";
		char* p;

		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIGrayOffset[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIGrayOffset[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
		//获取第二个颜色区域
		i = 0;
		rapidxml::xml_node<>* vertexs_blue = vertexs->first_node("blue");
		vertex = vertexs_blue->first_node();
		whitePoint = vertex->first_node();
		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIBlueOffset[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIBlueOffset[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
		//获取第三个颜色区域
		i = 0;
		rapidxml::xml_node<>* vertexs_yellow = vertexs->first_node("yellow");
		vertex = vertexs_yellow->first_node();
		whitePoint = vertex->first_node();
		p = strtok(whitePoint->value(), d);
		while (p)
		{
			float tmp = stof(p);
			ROIYellowOffset[i] = tmp;
			i += 1;
			p = strtok(NULL, d);

		}
		while (vertex->next_sibling())
		{
			vertex = vertex->next_sibling();
			whitePoint = vertex->first_node();
			char* q = strtok(whitePoint->value(), d);
			while (q)
			{
				float tmp = stof(q);
				ROIYellowOffset[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}

	}
	catch (rapidxml::parse_error e)
	{
		std::cout << e.what() << std::endl;
	}
	return;
}

void AWBAlgo::readWeightXML(char data[], size_t len)
{
	const int buf_len = 2048;
	char buf[buf_len] = { 0 };
	int i = 0;
	try
	{
		memset(buf, 0, buf_len);
		rapidxml::xml_document<> doc;
		doc.parse<0>(data);
		rapidxml::xml_node<>* root = doc.first_node();
		rapidxml::xml_node<>* node = root->first_node();
		rapidxml::xml_node<>* weight = node->first_node();
		const char* d = ",";
		char* p;

		p = strtok(weight->value(), d);
		while (p)
		{
			float tmp = stof(p);
			weights[i] = tmp;
			i += 1;
			p = strtok(NULL, d);
		}
		while (node->next_sibling())
		{
			node = node->next_sibling();
			weight = node->first_node();
			char* q = strtok(weight->value(), d);
			while (q)
			{
				float tmp = stof(q);
				weights[i] = tmp;
				i += 1;
				q = strtok(NULL, d);
			}
		}
	}
	catch (rapidxml::parse_error e)
	{
		std::cout << e.what() << std::endl;
	}
}

void AWBAlgo::config(void* ptr)
{
#if !BinaryConfig
	char* buffer_ = nullptr;
	FILE* fp;
	size_t sz;

	const char* file_name = "lightsourceConfig.xml";
	{
		basic_ifstream<char> stream(file_name, ios::binary);
		CV_Assert(stream);
		stream.unsetf(ios::skipws);
		stream.seekg(0, ios::end);
		sz = stream.tellg();
		stream.seekg(0);
		buffer_ = (char*)malloc(sz + 1);
		stream.read(buffer_, static_cast<streamsize>(sz));
		buffer_[sz] = 0;
	}
	fp = fopen("lightsourceConfig.txt", "w");
	fprintf(fp, "%s", "static char lightsourceConfig_bin[] = {");
	for (int i = 0; i < sz; i++)
	{
		int s = buffer_[i];
		CV_Assert(s >= 0 && s <= 127);
		fprintf(fp, "%d, ", s);
	}
	fprintf(fp, "%d", 0);
	fprintf(fp, "%s", "};");
	fclose(fp);
	free(buffer_);
	readLightXML(file_name);

	file_name = "weightConfig.xml";
	{
		basic_ifstream<char> stream(file_name, ios::binary);
		CV_Assert(stream);
		stream.unsetf(ios::skipws);
		stream.seekg(0, ios::end);
		sz = stream.tellg();
		stream.seekg(0);
		buffer_ = (char*)malloc(sz + 1);
		stream.read(buffer_, static_cast<streamsize>(sz));
		buffer_[sz] = 0;
	}
	fp = fopen("weightConfig.txt", "w");
	fprintf(fp, "%s", "static char weightConfig_bin[] = {");
	for (int i = 0; i < sz; i++)
	{
		int s = buffer_[i];
		CV_Assert(s >= 0 && s <= 127);
		fprintf(fp, "%d, ", s);
	}
	fprintf(fp, "%d", 0);
	fprintf(fp, "%s", "};");
	fclose(fp);
	free(buffer_);
	readWeightXML(file_name);

	file_name = "ROIConfig.xml";
	{
		basic_ifstream<char> stream(file_name, ios::binary);
		CV_Assert(stream);
		stream.unsetf(ios::skipws);
		stream.seekg(0, ios::end);
		sz = stream.tellg();
		stream.seekg(0);
		buffer_ = (char*)malloc(sz + 1);
		stream.read(buffer_, static_cast<streamsize>(sz));
		buffer_[sz] = 0;
	}
	fp = fopen("ROIConfig.txt", "w");
	fprintf(fp, "%s", "static char ROIConfig_bin[] = {");
	for (int i = 0; i < sz; i++)
	{
		int s = buffer_[i];
		CV_Assert(s >= 0 && s <= 127);
		fprintf(fp, "%d, ", s);
	}
	fprintf(fp, "%d", 0);
	fprintf(fp, "%s", "};");
	fclose(fp);
	free(buffer_);
	readROIXML(file_name);

	file_name = "ROIOffsetConfig.xml";
	{
		basic_ifstream<char> stream(file_name, ios::binary);
		CV_Assert(stream);
		stream.unsetf(ios::skipws);
		stream.seekg(0, ios::end);
		sz = stream.tellg();
		stream.seekg(0);
		buffer_ = (char*)malloc(sz + 1);
		stream.read(buffer_, static_cast<streamsize>(sz));
		buffer_[sz] = 0;
	}
	fp = fopen("ROIOffsetConfig.txt", "w");
	fprintf(fp, "%s", "static char ROIOffsetConfig_bin[] = {");
	for (int i = 0; i < sz; i++)
	{
		int s = buffer_[i];
		CV_Assert(s >= 0 && s <= 127);
		fprintf(fp, "%d, ", s);
	}
	fprintf(fp, "%d", 0);
	fprintf(fp, "%s", "};");
	fclose(fp);
	free(buffer_);
	readROIOffsetXML(file_name);

	file_name = "seriesConfig.xml";
	{
		basic_ifstream<char> stream(file_name, ios::binary);
		CV_Assert(stream);
		stream.unsetf(ios::skipws);
		stream.seekg(0, ios::end);
		sz = stream.tellg();
		stream.seekg(0);
		buffer_ = (char*)malloc(sz + 1);
		stream.read(buffer_, static_cast<streamsize>(sz));
		buffer_[sz] = 0;
	}
	fp = fopen("seriesConfig.txt", "w");
	fprintf(fp, "%s", "static char seriesConfig_bin[] = {");
	for (int i = 0; i < sz; i++)
	{
		int s = buffer_[i];
		CV_Assert(s >= 0 && s <= 127);
		fprintf(fp, "%d, ", s);
	}
	fprintf(fp, "%d", 0);
	fprintf(fp, "%s", "};");
	fclose(fp);
	free(buffer_);
	readSeriesXML(file_name);
#else
	char* buffer_ = nullptr;
	buffer_ = (char*)malloc(sizeof(lightsourceConfig_bin));
	memcpy(buffer_, lightsourceConfig_bin, sizeof(lightsourceConfig_bin));
	readLightXML(buffer_, sizeof(buffer_));
	free(buffer_);

	buffer_ = (char*)malloc(sizeof(weightConfig_bin));
	memcpy(buffer_, weightConfig_bin, sizeof(weightConfig_bin));
	readWeightXML(buffer_, sizeof(buffer_));
	free(buffer_);

	buffer_ = (char*)malloc(sizeof(ROIConfig_bin));
	memcpy(buffer_, ROIConfig_bin, sizeof(ROIConfig_bin));
	readROIXML(buffer_, sizeof(buffer_));
	free(buffer_);

	buffer_ = (char*)malloc(sizeof(ROIOffsetConfig_bin));
	memcpy(buffer_, ROIOffsetConfig_bin, sizeof(ROIOffsetConfig_bin));
	readROIOffsetXML(buffer_, sizeof(buffer_));
	free(buffer_);

	buffer_ = (char*)malloc(sizeof(seriesConfig_bin));
	memcpy(buffer_, seriesConfig_bin, sizeof(seriesConfig_bin));
	readSeriesXML(buffer_, sizeof(buffer_));
	free(buffer_);

#endif
}

bool AWBAlgo::roi_valid(float rg, float bg, vector<float> &valid_gray, vector<float> &valid_blue, vector<float> &valid_yellow)
{
    if(rg < ROIList[0] || rg > ROIList[6] || bg < ROIList[3] || bg > ROIList[5])
        return false;
	bool valid = false;
    if(rg > ROIGrayList[0] && rg < ROIGrayList[6] && bg > ROIGrayList[3] && bg < ROIGrayList[5])
	{
		valid_gray.push_back(rg);
		valid_gray.push_back(bg);
		valid = true;
    }
    if(rg > ROIBlueList[0] && rg < ROIBlueList[6] && bg > ROIBlueList[3] && bg < ROIBlueList[5])
	{
		valid_blue.push_back(rg);
		valid_blue.push_back(bg);
		valid = true;
    }
    if(rg > ROIYellowList[0] && rg < ROIYellowList[6] && bg > ROIYellowList[3] && bg < ROIYellowList[5])
	{
		valid_yellow.push_back(rg);
		valid_yellow.push_back(bg);
		valid = true;
    }
    return valid;
}

float AWBAlgo::distance(float rg, float bg, float stdA, float stdB)
{
    return pow(rg-stdA,2) + pow(bg-stdB,2);
}

void AWBAlgo::serial_filter(float final_gain[2])
{
	if (gain_series.size() < 8)
		return;
	while (gain_series.size() >= 9)
		gain_series.erase(gain_series.begin());
	for (int n = 0; n < gain_series.size(); n++)
	{
		final_gain[0] = serial_coef[1] * gain_series[0] + serial_coef[2] * gain_series[2] + serial_coef[3] * gain_series[4] + serial_coef[4] * gain_series[6];
		final_gain[1] = serial_coef[1] * gain_series[1] + serial_coef[2] * gain_series[3] + serial_coef[3] * gain_series[5] + serial_coef[4] * gain_series[7];
	}
}

void AWBAlgo::ROIProcess(vector<float> &valid_color, float color_gain[2])
{
    color_gain[0] = 0;
    color_gain[1] = 0;
	int valid_num = valid_color.size();
	if (valid_num == 0)
		return;
	float weightlist[4];
    vector<PAIR> dist_vec(4);
	memset(weightlist, 0, sizeof(weightlist));
	for(int n = 0; n < valid_num; n += 2)
	{
        for(int z = 0; z < 4 ; z++)
			dist_vec[z] = PAIR(z, distance(valid_color[n], valid_color[n + 1], whitepoints[2 * z], whitepoints[2 * z + 1]));
        sort(dist_vec.begin(), dist_vec.end(), cmpByValue());

		float dist1 = dist_vec[0].second; int wpid1 = dist_vec[0].first;
		float dist2 = dist_vec[1].second; int wpid2 = dist_vec[1].first;
		float weight1 = dist2 / (dist1 + dist2);
		float weight2 = 1 - weight1;
		weightlist[wpid1] += weight1;
		weightlist[wpid2] += weight2;

    }
	for (int wpid = 0; wpid < 4; wpid++)
		weightlist[wpid] *= weights[wpid];
	double weightsum = 1 / (weightlist[0] + weightlist[1] + weightlist[2] + weightlist[3]);
	for (int wpid = 0; wpid < 4; wpid++)
	{
		double wt = weightlist[wpid] * weightsum;
		color_gain[0] += wt * whitepoints[2 * wpid];
		color_gain[1] += wt * whitepoints[2 * wpid + 1];
	}
}

void AWBAlgo::estimate()
{
	memset(final_ccm, 0, sizeof(final_ccm));
	memset(final_gain, 0, sizeof(final_gain));
	int lsctotal = lscsz * 3;
	memset(final_lsctable, 0, lsctotal * sizeof(float));

	vector<float> valid_gray, valid_blue, valid_yellow;
	int valid_count = 0;
	float gray_gain[2];
    float yellow_gain[2]; 
    float blue_gain[2];

    for(int i = 0; i < blknum; i++)
	{
		int g = stats[i * 3 + 1] >> 1;
		if (g == 0)
			continue;
		float rg = static_cast<double>(stats[i * 3 + 2]) / g;
		float bg = static_cast<double>(stats[i * 3]) / g;
		if (roi_valid(rg, bg, valid_gray, valid_blue, valid_yellow))
			valid_count++;
    }
	if (valid_count < 100)
	{
		final_gain[0] = whitepoints[2];
		final_gain[1] = whitepoints[3];
		memcpy(final_ccm, ccm_list[1], sizeof(final_ccm));
		memcpy(final_lsctable, lsctable[1], lsctotal * sizeof(float));
		return;
	}

    ROIProcess(valid_gray, gray_gain);
    ROIProcess(valid_blue, blue_gain);
    ROIProcess(valid_yellow, yellow_gain);

    //分区域调整后加权平均
	int valid_gray_num = valid_gray.size();
	int valid_blue_num = valid_blue.size();
	int valid_yellow_num = valid_yellow.size();
    int total_num = valid_gray_num + valid_blue_num + valid_yellow_num;
    float weighted_gray_RGain = (ROIGrayOffset[0] + gray_gain[0]) * (1 + ROIGrayOffset[2]);
    float weighted_gray_BGain = (ROIGrayOffset[1] + gray_gain[1]) * (1 + ROIGrayOffset[2]);
    float weighted_blue_RGain = (ROIBlueOffset[0] + blue_gain[0]) * (1 + ROIBlueOffset[2]);
    float weighted_blue_BGain = (ROIBlueOffset[1] + blue_gain[1]) * (1 + ROIBlueOffset[2]);
    float weighted_yellow_RGain = (ROIYellowOffset[0] + yellow_gain[0]) * (1 + ROIYellowOffset[2]);
    float weighted_yellow_BGain = (ROIYellowOffset[1] + yellow_gain[1]) * (1 + ROIYellowOffset[2]);
    final_gain[0] = (weighted_gray_RGain * valid_gray_num + weighted_blue_RGain * valid_blue_num + weighted_yellow_RGain * valid_yellow_num) / total_num;
    final_gain[1] = (weighted_gray_BGain * valid_gray_num + weighted_blue_BGain * valid_blue_num + weighted_yellow_BGain * valid_yellow_num) / total_num;

    vector<PAIR> dist_vec(4);
    for(int z = 0; z < 4 ; z++)
        dist_vec[z] = PAIR(z, distance(final_gain[0], final_gain[1], whitepoints[2 * z], whitepoints[2 * z + 1]));
    sort(dist_vec.begin(), dist_vec.end(), cmpByValue());

    float dist1 = dist_vec[0].second; int wpid1 = dist_vec[0].first;
    float dist2 = dist_vec[1].second; int wpid2 = dist_vec[1].first;
    float weight1 = dist2 / (dist1 + dist2);
    float weight2 = 1 - weight1;

    for (int i = 0; i < 9; i++)
        final_ccm[i] = weight1 * ccm_list[wpid1][i] + weight2 * ccm_list[wpid2][i];

    for (int w=0; w < lsctotal; w++)
        final_lsctable[w] = weight1 * lsctable[wpid1][w] + weight2 * lsctable[wpid2][w];

	gain_series.push_back(final_gain[0]);
	gain_series.push_back(final_gain[1]);
	serial_filter(final_gain);
}

static float A_lsc0[] = { 11.633,9.287,7.468,6.085,5.060,4.330,3.844,3.569,3.483,3.580,3.867,4.366,5.113,6.157,7.564,9.412,11.792,
9.892,7.740,6.089,4.848,3.641,3.259,2.942,2.735,2.691,2.764,2.977,3.278,3.646,4.918,6.184,7.864,10.053,
8.569,6.576,5.063,3.656,3.137,2.641,2.253,2.033,1.965,2.056,2.285,2.672,3.160,3.658,5.156,6.700,8.731,
7.602,5.733,3.888,3.257,2.615,2.070,1.714,1.521,1.467,1.537,1.737,2.116,2.661,3.281,3.922,5.857,7.766,
6.941,5.161,3.607,2.918,2.208,1.687,1.379,1.226,1.184,1.240,1.401,1.736,2.273,2.963,3.580,5.288,7.109,
6.551,4.827,3.441,2.673,1.956,1.479,1.207,1.079,1.045,1.092,1.234,1.527,2.035,2.736,3.434,4.958,6.726,
6.412,4.708,3.357,2.585,1.875,1.407,1.150,1.030,1.000,1.047,1.183,1.461,1.956,2.664,3.382,4.845,6.595,
6.515,4.797,3.455,2.674,1.950,1.466,1.195,1.068,1.034,1.082,1.226,1.520,2.035,2.754,3.473,4.944,6.711,
6.866,5.100,3.651,2.908,2.174,1.653,1.346,1.199,1.156,1.215,1.380,1.711,2.268,2.973,3.685,5.260,7.079,
7.485,5.635,3.947,3.257,2.574,2.009,1.651,1.463,1.414,1.482,1.685,2.075,2.644,3.306,3.965,5.812,7.718,
8.405,6.437,4.946,3.639,3.096,2.549,2.155,1.936,1.871,1.958,2.194,2.607,3.121,3.683,5.095,6.636,8.664,
9.671,7.552,5.929,4.711,3.615,3.196,2.855,2.627,2.559,2.655,2.874,3.224,3.603,4.838,6.100,7.777,9.963,
11.345,9.040,7.256,5.901,4.899,4.187,3.716,3.452,3.373,3.474,3.763,4.262,5.008,6.050,7.453,9.298,11.675, };

static float A_lsc1[] = { 7.436,6.100,5.050,4.240,3.630,3.190,2.894,2.725,2.673,2.734,2.912,3.217,3.669,4.292,5.119,6.188,7.547,
6.415,5.179,4.216,3.479,2.756,2.498,2.295,2.165,2.128,2.185,2.321,2.523,2.794,3.529,4.282,5.265,6.524,
5.635,4.481,3.588,2.764,2.413,2.099,1.857,1.718,1.668,1.732,1.880,2.120,2.423,2.783,3.652,4.564,5.741,
5.061,3.971,2.928,2.484,2.081,1.741,1.505,1.372,1.335,1.385,1.520,1.768,2.119,2.498,2.963,4.052,5.166,
4.667,3.624,2.700,2.257,1.813,1.480,1.272,1.163,1.134,1.174,1.287,1.511,1.865,2.287,2.695,3.705,4.772,
4.436,3.420,2.575,2.093,1.654,1.336,1.148,1.059,1.036,1.069,1.167,1.367,1.705,2.137,2.588,3.502,4.542,
4.355,3.350,2.541,2.038,1.592,1.286,1.111,1.024,1.000,1.036,1.131,1.323,1.645,2.097,2.548,3.434,4.465,
4.421,3.408,2.596,2.103,1.653,1.330,1.147,1.055,1.030,1.062,1.167,1.368,1.703,2.151,2.604,3.496,4.536,
4.637,3.598,2.747,2.258,1.805,1.464,1.256,1.151,1.120,1.163,1.281,1.505,1.864,2.301,2.772,3.691,4.758,
5.013,3.930,2.966,2.495,2.070,1.707,1.471,1.340,1.303,1.354,1.494,1.746,2.116,2.514,3.003,4.031,5.144,
5.568,4.423,3.539,2.764,2.393,2.056,1.808,1.661,1.618,1.676,1.828,2.096,2.422,2.779,3.624,4.534,5.710,
6.326,5.102,4.149,3.420,2.749,2.462,2.255,2.107,2.062,2.116,2.264,2.481,2.746,3.492,4.244,5.225,6.483,
7.321,6.000,4.962,4.162,3.561,3.128,2.837,2.672,2.623,2.685,2.864,3.170,3.621,4.244,5.069,6.137,7.495, };

static float A_lsc2[] = { 7.033,5.790,4.811,4.055,3.485,3.074,2.798,2.641,2.594,2.654,2.824,3.116,3.545,4.135,4.916,5.925,7.203,
6.073,4.922,4.024,3.335,2.651,2.403,2.216,2.083,2.081,2.131,2.264,2.475,2.719,3.412,4.126,5.054,6.240,
5.338,4.264,3.431,2.669,2.333,2.043,1.802,1.676,1.643,1.697,1.849,2.083,2.366,2.708,3.530,4.392,5.503,
4.797,3.782,2.797,2.385,2.017,1.692,1.478,1.347,1.311,1.367,1.501,1.744,2.076,2.425,2.858,3.909,4.961,
4.427,3.454,2.591,2.181,1.765,1.447,1.248,1.147,1.122,1.166,1.276,1.489,1.831,2.226,2.617,3.582,4.591,
4.208,3.262,2.477,2.024,1.606,1.307,1.131,1.051,1.033,1.068,1.161,1.349,1.675,2.099,2.516,3.391,4.376,
4.131,3.194,2.432,1.972,1.553,1.257,1.095,1.018,1.000,1.032,1.126,1.309,1.625,2.044,2.468,3.328,4.305,
4.192,3.248,2.481,2.030,1.610,1.305,1.138,1.048,1.030,1.065,1.163,1.357,1.677,2.111,2.548,3.389,4.374,
4.394,3.427,2.616,2.188,1.756,1.434,1.239,1.140,1.116,1.158,1.278,1.493,1.835,2.244,2.698,3.577,4.588,
4.746,3.739,2.825,2.403,2.000,1.668,1.445,1.319,1.287,1.342,1.477,1.726,2.083,2.460,2.907,3.902,4.956,
5.266,4.203,3.380,2.638,2.327,1.991,1.764,1.624,1.591,1.644,1.798,2.061,2.364,2.715,3.517,4.382,5.495,
5.977,4.841,3.955,3.276,2.617,2.373,2.162,2.029,1.995,2.047,2.204,2.396,2.653,3.393,4.108,5.040,6.230,
6.910,5.684,4.720,3.977,3.418,3.015,2.746,2.595,2.552,2.616,2.790,3.084,3.516,4.109,4.894,5.906,7.190, };

static float d65_lsc0[] = { 9.443,7.570,6.116,5.009,4.188,3.604,3.217,3.000,2.938,3.025,3.269,3.686,4.307,5.172,6.332,7.850,9.802,
8.124,6.394,5.065,4.063,3.085,2.780,2.529,2.349,2.303,2.384,2.575,2.859,3.183,4.212,5.265,6.657,8.463,
7.121,5.509,4.280,3.135,2.713,2.295,1.979,1.808,1.749,1.830,2.033,2.364,2.776,3.223,4.468,5.757,7.444,
6.390,4.868,3.383,2.836,2.306,1.856,1.562,1.400,1.353,1.423,1.604,1.934,2.409,2.928,3.516,5.105,6.701,
5.895,4.437,3.171,2.591,2.002,1.567,1.297,1.166,1.135,1.185,1.338,1.629,2.107,2.707,3.231,4.668,6.199,
5.612,4.193,3.049,2.415,1.811,1.397,1.165,1.053,1.026,1.076,1.209,1.473,1.918,2.530,3.139,4.421,5.914,
5.527,4.119,2.993,2.347,1.748,1.350,1.130,1.024,1.000,1.046,1.171,1.427,1.860,2.467,3.084,4.350,5.832,
5.635,4.213,3.063,2.421,1.820,1.409,1.173,1.066,1.039,1.083,1.220,1.486,1.938,2.558,3.167,4.450,5.948,
5.943,4.480,3.256,2.634,2.021,1.578,1.315,1.184,1.153,1.206,1.363,1.655,2.135,2.749,3.377,4.728,6.269,
6.465,4.935,3.525,2.941,2.364,1.892,1.586,1.427,1.382,1.450,1.636,1.985,2.482,3.033,3.633,5.199,6.810,
7.227,5.604,4.367,3.293,2.811,2.359,2.037,1.841,1.790,1.868,2.091,2.446,2.898,3.389,4.583,5.890,7.598,
8.265,6.523,5.182,4.171,3.289,2.915,2.623,2.440,2.381,2.472,2.677,2.976,3.331,4.349,5.421,6.836,8.668,
9.626,7.738,6.271,5.153,4.323,3.732,3.341,3.123,3.062,3.154,3.405,3.835,4.471,5.354,6.538,8.084,10.067, };

static float d65_lsc1[] = { 6.461,5.307,4.400,3.701,3.176,2.798,2.546,2.404,2.365,2.425,2.588,2.865,3.271,3.829,4.567,5.521,6.732,
5.634,4.559,3.722,3.082,2.470,2.230,2.051,1.945,1.916,1.969,2.099,2.290,2.548,3.198,3.876,4.758,5.886,
5.001,3.992,3.211,2.500,2.182,1.905,1.690,1.569,1.528,1.590,1.738,1.962,2.251,2.580,3.354,4.178,5.239,
4.537,3.579,2.680,2.276,1.912,1.608,1.401,1.286,1.254,1.301,1.434,1.660,1.990,2.343,2.802,3.755,4.763,
4.222,3.300,2.504,2.098,1.701,1.399,1.213,1.116,1.093,1.136,1.246,1.453,1.778,2.177,2.556,3.469,4.441,
4.043,3.142,2.396,1.963,1.567,1.285,1.121,1.040,1.022,1.058,1.151,1.336,1.644,2.059,2.476,3.306,4.256,
3.991,3.096,2.365,1.919,1.520,1.247,1.089,1.018,1.000,1.036,1.122,1.301,1.605,2.021,2.436,3.259,4.203,
4.063,3.160,2.410,1.977,1.573,1.291,1.129,1.049,1.029,1.062,1.158,1.346,1.656,2.071,2.497,3.324,4.277,
4.264,3.336,2.543,2.125,1.714,1.414,1.230,1.137,1.114,1.151,1.266,1.471,1.801,2.213,2.644,3.505,4.482,
4.601,3.636,2.764,2.345,1.957,1.639,1.427,1.310,1.280,1.328,1.457,1.696,2.038,2.425,2.864,3.812,4.827,
5.090,4.072,3.283,2.600,2.272,1.959,1.737,1.602,1.567,1.625,1.773,2.018,2.326,2.672,3.426,4.258,5.328,
5.753,4.667,3.819,3.170,2.593,2.335,2.140,2.016,1.982,2.034,2.174,2.374,2.632,3.286,3.973,4.865,6.005,
6.614,5.446,4.526,3.817,3.283,2.899,2.642,2.497,2.457,2.518,2.684,2.965,3.378,3.945,4.694,5.660,6.884, };

static float d65_lsc2[] = { 5.725,4.745,3.971,3.372,2.921,2.594,2.376,2.253,2.220,2.273,2.416,2.658,3.011,3.494,4.129,4.947,5.978,
5.006,4.092,3.376,2.826,2.303,2.094,1.931,1.824,1.813,1.866,1.992,2.160,2.401,2.939,3.524,4.282,5.246,
4.454,3.595,2.927,2.333,2.041,1.794,1.609,1.492,1.464,1.518,1.654,1.858,2.105,2.415,3.066,3.776,4.684,
4.049,3.232,2.471,2.120,1.801,1.533,1.346,1.239,1.215,1.260,1.379,1.587,1.878,2.192,2.587,3.406,4.271,
3.773,2.986,2.312,1.951,1.605,1.340,1.173,1.094,1.074,1.115,1.206,1.398,1.682,2.035,2.384,3.155,3.990,
3.615,2.846,2.211,1.828,1.483,1.237,1.094,1.030,1.017,1.046,1.127,1.289,1.565,1.917,2.288,3.013,3.830,
3.567,2.804,2.174,1.779,1.438,1.197,1.069,1.013,1.000,1.030,1.103,1.255,1.521,1.878,2.249,2.971,3.783,
3.627,2.857,2.219,1.830,1.486,1.235,1.100,1.038,1.024,1.052,1.136,1.298,1.567,1.934,2.297,3.029,3.848,
3.799,3.010,2.347,1.971,1.620,1.352,1.193,1.112,1.095,1.129,1.229,1.409,1.701,2.062,2.442,3.188,4.027,
4.089,3.269,2.548,2.169,1.834,1.554,1.364,1.260,1.237,1.283,1.402,1.616,1.913,2.256,2.657,3.457,4.329,
4.510,3.647,2.974,2.402,2.106,1.838,1.636,1.520,1.495,1.542,1.678,1.904,2.177,2.495,3.129,3.847,4.765,
5.080,4.161,3.441,2.887,2.385,2.156,1.988,1.876,1.833,1.892,2.019,2.210,2.444,3.015,3.609,4.378,5.353,
5.821,4.834,4.055,3.452,2.996,2.667,2.447,2.324,2.292,2.347,2.495,2.742,3.102,3.594,4.240,5.070,6.116, };

static float tl84_lsc0[] = { 6.961,5.691,4.695,3.927,3.350,2.933,2.653,2.492,2.441,2.495,2.660,2.944,3.365,3.947,4.721,5.725,7.003,
6.056,4.877,3.958,3.256,2.613,2.357,2.159,2.035,1.995,2.036,2.163,2.369,2.627,3.279,3.988,4.915,6.104,
5.362,4.257,3.403,2.630,2.281,1.977,1.743,1.611,1.563,1.622,1.755,1.989,2.278,2.631,3.435,4.300,5.417,
4.852,3.805,2.811,2.383,1.979,1.647,1.418,1.298,1.261,1.305,1.432,1.659,1.995,2.381,2.851,3.851,4.913,
4.504,3.497,2.634,2.176,1.750,1.421,1.224,1.124,1.099,1.130,1.232,1.440,1.783,2.208,2.621,3.548,4.570,
4.302,3.320,2.542,2.050,1.615,1.304,1.124,1.039,1.020,1.046,1.139,1.328,1.650,2.080,2.536,3.376,4.374,
4.236,3.262,2.493,1.998,1.565,1.264,1.099,1.018,1.000,1.028,1.114,1.288,1.609,2.032,2.504,3.324,4.316,
4.304,3.322,2.548,2.051,1.616,1.303,1.128,1.041,1.022,1.047,1.144,1.329,1.661,2.099,2.550,3.390,4.392,
4.508,3.503,2.662,2.190,1.746,1.413,1.225,1.127,1.104,1.133,1.241,1.447,1.796,2.232,2.693,3.579,4.607,
4.859,3.813,2.893,2.419,1.996,1.646,1.424,1.305,1.274,1.313,1.440,1.684,2.027,2.438,2.918,3.899,4.970,
5.371,4.269,3.417,2.686,2.326,1.994,1.756,1.621,1.579,1.632,1.776,2.025,2.346,2.704,3.492,4.367,5.497,
6.067,4.892,3.977,3.278,2.705,2.423,2.212,2.071,2.020,2.083,2.207,2.430,2.702,3.344,4.064,5.005,6.210,
6.976,5.712,4.720,3.955,3.381,2.968,2.690,2.533,2.486,2.545,2.716,3.007,3.438,4.032,4.821,5.841,7.139, };

static float tl84_lsc1[] = { 5.997,4.958,4.138,3.502,3.023,2.675,2.440,2.306,2.265,2.313,2.455,2.697,3.054,3.545,4.193,5.028,6.085,
5.245,4.277,3.518,2.934,2.393,2.177,2.003,1.902,1.882,1.921,2.029,2.194,2.429,2.975,3.571,4.345,5.332,
4.668,3.757,3.049,2.396,2.109,1.852,1.654,1.537,1.502,1.547,1.675,1.869,2.117,2.419,3.101,3.824,4.753,
4.243,3.378,2.564,2.183,1.853,1.568,1.374,1.269,1.236,1.274,1.384,1.593,1.884,2.203,2.610,3.444,4.328,
3.954,3.120,2.397,2.018,1.651,1.371,1.199,1.114,1.088,1.120,1.213,1.395,1.690,2.050,2.403,3.186,4.039,
3.787,2.972,2.317,1.903,1.533,1.266,1.114,1.042,1.020,1.048,1.128,1.288,1.569,1.943,2.333,3.039,3.873,
3.735,2.926,2.288,1.859,1.483,1.232,1.090,1.019,1.000,1.025,1.100,1.257,1.528,1.909,2.297,2.995,3.823,
3.795,2.980,2.324,1.914,1.531,1.265,1.122,1.043,1.021,1.049,1.130,1.299,1.579,1.954,2.344,3.052,3.888,
3.971,3.136,2.439,2.040,1.663,1.377,1.206,1.119,1.095,1.129,1.223,1.409,1.703,2.064,2.470,3.212,4.069,
4.270,3.403,2.626,2.236,1.873,1.574,1.382,1.280,1.245,1.286,1.400,1.613,1.918,2.259,2.671,3.485,4.375,
4.705,3.792,3.082,2.470,2.159,1.877,1.676,1.550,1.518,1.561,1.691,1.916,2.182,2.490,3.150,3.881,4.818,
5.294,4.323,3.562,2.976,2.478,2.243,2.078,1.947,1.910,1.951,2.078,2.254,2.481,3.034,3.638,4.421,5.418,
6.060,5.018,4.195,3.557,3.076,2.727,2.492,2.358,2.318,2.369,2.514,2.761,3.125,3.623,4.280,5.126,6.196, };

static float tl84_lsc2[] = { 5.687,4.721,3.957,3.364,2.916,2.591,2.371,2.247,2.209,2.256,2.392,2.622,2.961,3.425,4.036,4.821,5.813,
4.978,4.077,3.370,2.825,2.317,2.105,1.939,1.843,1.824,1.872,1.978,2.161,2.374,2.883,3.447,4.176,5.102,
4.432,3.585,2.924,2.320,2.054,1.798,1.615,1.504,1.467,1.522,1.643,1.844,2.077,2.369,3.000,3.683,4.557,
4.030,3.225,2.463,2.119,1.796,1.543,1.348,1.242,1.217,1.259,1.370,1.569,1.840,2.146,2.530,3.323,4.155,
3.755,2.979,2.304,1.950,1.608,1.349,1.182,1.101,1.077,1.113,1.197,1.381,1.662,1.992,2.335,3.078,3.882,
3.595,2.837,2.217,1.840,1.497,1.240,1.100,1.031,1.017,1.044,1.120,1.277,1.549,1.893,2.255,2.939,3.725,
3.543,2.792,2.183,1.789,1.448,1.207,1.071,1.015,1.000,1.029,1.097,1.242,1.500,1.854,2.217,2.897,3.679,
3.597,2.840,2.218,1.845,1.490,1.244,1.103,1.036,1.018,1.046,1.123,1.285,1.554,1.906,2.272,2.951,3.740,
3.759,2.985,2.321,1.957,1.607,1.350,1.185,1.105,1.086,1.122,1.212,1.382,1.682,2.014,2.387,3.103,3.911,
4.037,3.233,2.512,2.143,1.812,1.540,1.352,1.252,1.220,1.263,1.384,1.584,1.873,2.194,2.573,3.361,4.201,
4.442,3.597,2.938,2.367,2.085,1.826,1.627,1.513,1.483,1.532,1.655,1.879,2.124,2.430,3.047,3.738,4.621,
4.991,4.093,3.388,2.845,2.382,2.155,1.994,1.871,1.833,1.892,2.003,2.176,2.423,2.937,3.509,4.248,5.187,
5.704,4.742,3.981,3.390,2.944,2.621,2.404,2.282,2.248,2.299,2.439,2.676,3.022,3.495,4.117,4.915,5.922, };

static float high_lsc0[] = { 9.443,7.570,6.116,5.009,4.188,3.604,3.217,3.000,2.938,3.025,3.269,3.686,4.307,5.172,6.332,7.850,9.802,
8.124,6.394,5.065,4.063,3.085,2.780,2.529,2.349,2.303,2.384,2.575,2.859,3.183,4.212,5.265,6.657,8.463,
7.121,5.509,4.280,3.135,2.713,2.295,1.979,1.808,1.749,1.830,2.033,2.364,2.776,3.223,4.468,5.757,7.444,
6.390,4.868,3.383,2.836,2.306,1.856,1.562,1.400,1.353,1.423,1.604,1.934,2.409,2.928,3.516,5.105,6.701,
5.895,4.437,3.171,2.591,2.002,1.567,1.297,1.166,1.135,1.185,1.338,1.629,2.107,2.707,3.231,4.668,6.199,
5.612,4.193,3.049,2.415,1.811,1.397,1.165,1.053,1.026,1.076,1.209,1.473,1.918,2.530,3.139,4.421,5.914,
5.527,4.119,2.993,2.347,1.748,1.350,1.130,1.024,1.000,1.046,1.171,1.427,1.860,2.467,3.084,4.350,5.832,
5.635,4.213,3.063,2.421,1.820,1.409,1.173,1.066,1.039,1.083,1.220,1.486,1.938,2.558,3.167,4.450,5.948,
5.943,4.480,3.256,2.634,2.021,1.578,1.315,1.184,1.153,1.206,1.363,1.655,2.135,2.749,3.377,4.728,6.269,
6.465,4.935,3.525,2.941,2.364,1.892,1.586,1.427,1.382,1.450,1.636,1.985,2.482,3.033,3.633,5.199,6.810,
7.227,5.604,4.367,3.293,2.811,2.359,2.037,1.841,1.790,1.868,2.091,2.446,2.898,3.389,4.583,5.890,7.598,
8.265,6.523,5.182,4.171,3.289,2.915,2.623,2.440,2.381,2.472,2.677,2.976,3.331,4.349,5.421,6.836,8.668,
9.626,7.738,6.271,5.153,4.323,3.732,3.341,3.123,3.062,3.154,3.405,3.835,4.471,5.354,6.538,8.084,10.067, };

static float high_lsc1[] = { 6.461,5.307,4.400,3.701,3.176,2.798,2.546,2.404,2.365,2.425,2.588,2.865,3.271,3.829,4.567,5.521,6.732,
5.634,4.559,3.722,3.082,2.470,2.230,2.051,1.945,1.916,1.969,2.099,2.290,2.548,3.198,3.876,4.758,5.886,
5.001,3.992,3.211,2.500,2.182,1.905,1.690,1.569,1.528,1.590,1.738,1.962,2.251,2.580,3.354,4.178,5.239,
4.537,3.579,2.680,2.276,1.912,1.608,1.401,1.286,1.254,1.301,1.434,1.660,1.990,2.343,2.802,3.755,4.763,
4.222,3.300,2.504,2.098,1.701,1.399,1.213,1.116,1.093,1.136,1.246,1.453,1.778,2.177,2.556,3.469,4.441,
4.043,3.142,2.396,1.963,1.567,1.285,1.121,1.040,1.022,1.058,1.151,1.336,1.644,2.059,2.476,3.306,4.256,
3.991,3.096,2.365,1.919,1.520,1.247,1.089,1.018,1.000,1.036,1.122,1.301,1.605,2.021,2.436,3.259,4.203,
4.063,3.160,2.410,1.977,1.573,1.291,1.129,1.049,1.029,1.062,1.158,1.346,1.656,2.071,2.497,3.324,4.277,
4.264,3.336,2.543,2.125,1.714,1.414,1.230,1.137,1.114,1.151,1.266,1.471,1.801,2.213,2.644,3.505,4.482,
4.601,3.636,2.764,2.345,1.957,1.639,1.427,1.310,1.280,1.328,1.457,1.696,2.038,2.425,2.864,3.812,4.827,
5.090,4.072,3.283,2.600,2.272,1.959,1.737,1.602,1.567,1.625,1.773,2.018,2.326,2.672,3.426,4.258,5.328,
5.753,4.667,3.819,3.170,2.593,2.335,2.140,2.016,1.982,2.034,2.174,2.374,2.632,3.286,3.973,4.865,6.005,
6.614,5.446,4.526,3.817,3.283,2.899,2.642,2.497,2.457,2.518,2.684,2.965,3.378,3.945,4.694,5.660,6.884, };

static float high_lsc2[] = { 5.725,4.745,3.971,3.372,2.921,2.594,2.376,2.253,2.220,2.273,2.416,2.658,3.011,3.494,4.129,4.947,5.978,
5.006,4.092,3.376,2.826,2.303,2.094,1.931,1.824,1.813,1.866,1.992,2.160,2.401,2.939,3.524,4.282,5.246,
4.454,3.595,2.927,2.333,2.041,1.794,1.609,1.492,1.464,1.518,1.654,1.858,2.105,2.415,3.066,3.776,4.684,
4.049,3.232,2.471,2.120,1.801,1.533,1.346,1.239,1.215,1.260,1.379,1.587,1.878,2.192,2.587,3.406,4.271,
3.773,2.986,2.312,1.951,1.605,1.340,1.173,1.094,1.074,1.115,1.206,1.398,1.682,2.035,2.384,3.155,3.990,
3.615,2.846,2.211,1.828,1.483,1.237,1.094,1.030,1.017,1.046,1.127,1.289,1.565,1.917,2.288,3.013,3.830,
3.567,2.804,2.174,1.779,1.438,1.197,1.069,1.013,1.000,1.030,1.103,1.255,1.521,1.878,2.249,2.971,3.783,
3.627,2.857,2.219,1.830,1.486,1.235,1.100,1.038,1.024,1.052,1.136,1.298,1.567,1.934,2.297,3.029,3.848,
3.799,3.010,2.347,1.971,1.620,1.352,1.193,1.112,1.095,1.129,1.229,1.409,1.701,2.062,2.442,3.188,4.027,
4.089,3.269,2.548,2.169,1.834,1.554,1.364,1.260,1.237,1.283,1.402,1.616,1.913,2.256,2.657,3.457,4.329,
4.510,3.647,2.974,2.402,2.106,1.838,1.636,1.520,1.495,1.542,1.678,1.904,2.177,2.495,3.129,3.847,4.765,
5.080,4.161,3.441,2.887,2.385,2.156,1.988,1.876,1.833,1.892,2.019,2.210,2.444,3.015,3.609,4.378,5.353,
5.821,4.834,4.055,3.452,2.996,2.667,2.447,2.324,2.292,2.347,2.495,2.742,3.102,3.594,4.240,5.070,6.116, };

AWBAlgo::AWBAlgo(int lscsz_)
	: lscsz(lscsz_)
{
	int lsctotal = lscsz * 3;
	table_buffer = (float*)malloc(5 * lsctotal * sizeof(float));
	float* ptr = table_buffer;
	final_lsctable = ptr;
	ptr += lsctotal;
	for (int i = 0; i < 4; i++)
	{
		lsctable[i] = ptr;
		ptr += lsctotal;
	}
	memcpy(lsctable[0], high_lsc0, lscsz * sizeof(float));
	memcpy(lsctable[0] + lscsz, high_lsc1, lscsz * sizeof(float));
	memcpy(lsctable[0] + lscsz * 2, high_lsc2, lscsz * sizeof(float));

	memcpy(lsctable[1], d65_lsc0, lscsz * sizeof(float));
	memcpy(lsctable[1] + lscsz, d65_lsc1, lscsz * sizeof(float));
	memcpy(lsctable[1] + lscsz * 2, d65_lsc2, lscsz * sizeof(float));

	memcpy(lsctable[2], tl84_lsc0, lscsz * sizeof(float));
	memcpy(lsctable[2] + lscsz, tl84_lsc1, lscsz * sizeof(float));
	memcpy(lsctable[2] + lscsz * 2, tl84_lsc2, lscsz * sizeof(float));

	memcpy(lsctable[3], A_lsc0, lscsz * sizeof(float));
	memcpy(lsctable[3] + lscsz, A_lsc1, lscsz * sizeof(float));
	memcpy(lsctable[3] + lscsz * 2, A_lsc2, lscsz * sizeof(float));
}

AWBAlgo::~AWBAlgo()
{
	if (table_buffer)
		free(table_buffer);
}