// File utilities

#include <vector>
#include <fstream>
#include <string>

#include "file.hpp"

std::vector<char> app::read_file(std::string filename)
{
	std::vector<char> buffer;
	size_t size;

	std::ifstream file(filename, std::ios::in | std::ios::ate | std::ios::binary);
	if (file.is_open())
	{
		size = file.tellg();
		file.seekg(0);
		buffer.resize(size);
		file.read(buffer.data(), size);
		file.close();
	}
	else
	{
		throw std::runtime_error("Could not open file!");
	}

	return buffer;
}