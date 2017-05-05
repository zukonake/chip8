#pragma once

#include <string>
//
#include <typedef.hpp>

class Chip8
{
public:
	Chip8();
	
	void load(std::string const &name);
	void emulate();
private:
	Opcode fetchOpcode() const;
	void emulateCycle();
	void executeCurrentOpcode();
	void executeOpcode( Opcode const &opcode );
	void updateTimers();
	
	void loadFontSet();
	
	void clearMemory();
	void clearRegisters();
	void clearStack();
	void clearScreen();
	void clearKeypad();
	
	Key getKey();
	void setKeypad();
	
	void beep();
	
	bool emulating;
	
	Byte memory[0x1000];
	Byte registers[0x10];
	
	Address addressRegister;
	Address programCounter;
	
	Address stack[0x10];
	Address stackPointer;
	
	Timer delayTimer;
	Timer soundTimer;
	
	Pixel screen[0x40 * 0x20];
	bool keypad[0x10];
	
	/* should be around 17, but 18 gives better results */
	const double clockPeriod = 18;
	
	const Byte fontSet[0x10 * 0x5] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
};
