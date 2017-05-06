#include <iostream>
//
#include <chip8.hpp>

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " filename\n";
		return 0;
	}
	Chip8 chip8;
	chip8.load(argv[1]);
	chip8.emulate();
	return 0;
}
