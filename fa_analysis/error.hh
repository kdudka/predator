#pragma once

#include <sstream>
#include <iomanip>
#include <stdexcept>

class Error {

	static void error(const std::string& file, int line, const std::stringstream& msg) {
		std::stringstream os;
		os << '[' <<  file << "] error at line " << line << ": " << msg.str();
		throw std::runtime_error(os.str());
	}

public:

	static void eofUnexpected(const std::string& file, int line) {
		std::stringstream os;
		os << "unexpected end of file";
		Error::error(file, line, os);
	}

	static void invalidChar(const std::string& file, int line, char c) {
		std::stringstream os;
		os << "invalid character (0x" << std::setw(2) << std::setfill('0') << std::hex << c << ')';
		Error::error(file, line, os);
	}

	static void tokenUnexpected(const std::string& file, int line, const std::string& exp, const std::string& got) {
		std::stringstream os;
		os << "expected " << exp << ", got " << got;
		Error::error(file, line, os);
	}

	static void general(const std::string& file, int line, const std::string& msg) {
		std::stringstream os;
		os << msg;
		Error::error(file, line, os);
	}

};
