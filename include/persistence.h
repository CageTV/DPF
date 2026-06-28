#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

bool LoadGlobalRegistry();
bool SaveGlobalRegistry();
std::string GetGlobalRegistryPath();
