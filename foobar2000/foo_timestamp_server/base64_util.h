#pragma once

namespace base64_util {
	// Decode Base64 encoded string
	std::string decode(const std::string &val);

	// Encode using Base64
	std::string encode(const std::string &val);
}