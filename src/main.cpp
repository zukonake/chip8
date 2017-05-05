#include <chip8.hpp>

int main()
{
	Chip8 chip8;
	chip8.load("pong");
	chip8.emulate();
	return 0;
}
