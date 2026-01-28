#pragma once

#include <functional>

void runInBackground(std::function<void()> task);
