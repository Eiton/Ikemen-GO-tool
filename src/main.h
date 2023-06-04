#ifndef HEADERFILE_MAIN
#define HEADERFILE_MAIN
#include <imgui.h>
#include <windows.h>
#include <future>
#include "char.h"
#include "utility.h"

Char character;
bool isLoading;
std::future<Utility::ExecutionResult<int>> loadingResult;
std::string popupMessage = "";
#endif