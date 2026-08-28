#pragma once
#include <functional>
#include <string>

bool GenerateAssets(const std::string& root, const std::function<void(int, const std::string&)>& progress);
