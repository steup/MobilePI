#pragma once

#include <cstdint>

enum class TimeUnit {
	us, ms, s
};

void convert(TimeUnit a, TimeUnit b, uint32_t& nom, uint32_t& denom);
