#pragma once

#include "../witutils/diff_utils.h"
#include "../witutils/diff_commons.h"
#include "../witutils/str_utils.h"
#include "../witutils/file_utils.h"

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <set>
#include <filesystem>

#define VC_EXTRALEAN
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
