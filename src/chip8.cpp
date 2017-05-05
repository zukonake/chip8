#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
//
#include <chip8.hpp>

Chip8::Chip8() :
	emulating(false),
	addressRegister(0x0),
	programCounter(0x200),
	stackPointer(0x0),
	delayTimer(0x0),
	soundTimer(0x0)
{
	srand(time(NULL));
	clearMemory();
	clearRegisters();
	clearStack();
	clearScreen();
	clearKeypad();
	loadFontSet();
	std::cout << "INFO: created Chip8 object\n";
}

void Chip8::load(std::string const &name)
{
	//TODO
}

void Chip8::emulate()
{
	std::cout << "INFO: starting Chip8 emulation\n";
	std::chrono::system_clock::time_point before =
		std::chrono::system_clock::now();
	std::chrono::system_clock::time_point after =
		std::chrono::system_clock::now();
	emulating = true;
	while(emulating)
	{
		before = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> delta = before - after;
		if(delta.count() < clockPeriod)
		{
			std::chrono::duration<double, std::milli> deltaMilliSeconds(
				clockPeriod - delta.count());
			auto deltaMilliSecondsDuration = std::chrono::duration_cast<
				std::chrono::milliseconds>(deltaMilliSeconds);
			std::this_thread::sleep_for(std::chrono::milliseconds(
				deltaMilliSecondsDuration.count()));
		}
		after = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleepTime = after - before;
		emulateCycle();
		std::cout << "INFO: clock " << (delta + sleepTime).count() << "\n";
	}
}

Opcode Chip8::fetchOpcode() const
{
	std::cout << "INFO: fetching opcode at "
			  << std::hex << programCounter << "\n";
	return (memory[programCounter] << 0x8) | memory[programCounter + 0x1];
}

void Chip8::emulateCycle()
{
	setKeypad();
	executeCurrentOpcode();
	updateTimers();
}

void Chip8::executeCurrentOpcode()
{
	executeOpcode(fetchOpcode());
}

void Chip8::executeOpcode(Opcode const &opcode)
{
	std::cout << "INFO: executing opcode "
			  << std::hex << opcode << "\n";
	uint8_t x = (opcode & 0x0F00) >> 0x8;
	uint8_t y = (opcode & 0x00F0) >> 0x4;
	switch(opcode & 0xF000)
	{
	case 0x0000:
		switch(opcode & 0x000F)
		{
		case 0x0000:
			clearScreen();
			break;
		
		case 0x000E:
			programCounter = stack[stackPointer];
			stackPointer -= 0x1;
			break;
		
		default:
			//TODO
			break;
		}
		break;
	
	case 0x1000:
		programCounter = opcode & 0x0FFF;
		break;
	
	case 0x2000:
		stack[stackPointer] = programCounter;
		stackPointer += 0x1;
		programCounter = opcode & 0x0FFF;
		break;
	
	case 0x3000:
		if(registers[x] == (opcode & 0x00FF))
		{
			programCounter += 0x2;
		}
		break;
	
	case 0x4000:
		if(registers[x] != (opcode & 0x00FF))
		{
			programCounter += 0x2;
		}
		break;
	
	case 0x5000:
		if(registers[x] == registers[y])
		{
			programCounter += 0x2;
		}
		break;
	
	case 0x6000:
		registers[x] = opcode & 0x00FF;
		break;
	
	case 0x7000:
		registers[x] += opcode & 0x00FF;
		break;
	
	case 0x8000:
		switch(opcode & 0x000F)
		{
		case 0x0000:
			registers[x] = registers[y];
			break;
		
		case 0x0001:
			registers[x] |= registers[y];
			break;
		
		case 0x0002:
			registers[x] &= registers[y];
			break;
		
		case 0x0003:
			registers[x] ^= registers[y];
			break;
		
		case 0x0004:
			if(registers[x] > (0xFF - registers[y]))
			{
				registers[0xF] = 0x1;
			}
			else
			{
				registers[0xF] = 0x0;
			}
			registers[x] += registers[y];
			break;
		
		case 0x0005:
			if(registers[x] > registers[y])
			{
				registers[0xF] = 0x1;
			}
			else
			{
				registers[0xF] = 0x0;
			}
			registers[x] -= registers[y];
			break;
		
		case 0x0006:
			if(registers[x] % 0x2)
			{
				registers[0xF] = 0x1;
			}
			else
			{
				registers[0xF] = 0x0;
			}
			registers[x] /= 0x2;
			break;
		
		case 0x0007:
			if(registers[x] < registers[y])
			{
				registers[0xF] = 0x1;
			}
			else
			{
				registers[0xF] = 0x0;
			}
			registers[x] = registers[y] - registers[x];
			break;
		
		case 0x000E:
			if(registers[x] >> uint16_t(log2(registers[x])))
			{
				registers[0xF] = 0x1;
			}
			else
			{
				registers[0xF] = 0x0;
			}
			registers[x] *= 0x2;
			break;
		}
		break;
	
	case 0x9000:
		if(registers[x] != registers[y])
		{
			programCounter += 0x2;
		}
		break;
	
	case 0xA000:
		addressRegister = opcode & 0x0FFF;
		break;
	
	case 0xB000:
		programCounter = (opcode & 0x0FFF) + registers[0x0];
		break;
	
	case 0xC000:
		registers[x] = (rand() % 0x100) & (opcode & 0x00FF);
		break;
	
	case 0xD000:
		//TODO
		break;
	
	case 0xE000:
		if(keypad[registers[x]])
		{
			programCounter += 0x2;
		}
		break;
	
	case 0xF000:
		switch( opcode & 0x00FF )
		{
		case 0x0007:
			registers[x] = delayTimer;
			break;
		
		case 0x000A:
			registers[x] = getKey();
			break;
		
		case 0x0015:
			delayTimer = registers[x];
			break;
		
		case 0x0018:
			soundTimer = registers[x];
			break;
		
		case 0x001E:
			addressRegister += registers[x];
			break;
		
		case 0x0029:
			addressRegister = registers[x] * 0x5;
			break;
		
		case 0x0033:
			memory[addressRegister] = (registers[x] / 100) % 10;
			memory[addressRegister + 0x1] = (registers[x] / 10) % 10;
			memory[addressRegister + 0x2] = registers[x] % 10;
			break;
		
		case 0x0055:
			for(uint8_t i = 0x0; i <= x; i++)
			{
				memory[addressRegister + i] = registers[i];
			}
			break;
		
		case 0x0065:
			for(uint8_t i = 0x0; i <= x; i++)
			{
				registers[i] = memory[addressRegister + i];
			}
			break;
		}
		break;
	default:
		//TODO
		break;
	}
	programCounter += 0x2;
}

void Chip8::updateTimers()
{
	if(delayTimer > 0)
	{
		delayTimer -= 1;
	}
	if(soundTimer > 0)
	{
		soundTimer -= 1;
		if(soundTimer == 0)
		{
			beep();
		}
	}
}

void Chip8::loadFontSet()
{
	for(uint16_t i = 0x0; i < 0x50; i++)
	{
		memory[i] = fontSet[i];
	}
}

void Chip8::clearMemory()
{
	for(uint16_t i = 0x0; i < 0x1000; i++)
	{
		memory[i] = 0x0;
	}
}

void Chip8::clearRegisters()
{
	for(uint8_t i = 0x0; i < 0x10; i++)
	{
		registers[i] = 0x0;
	}
}

void Chip8::clearStack()
{
	for(uint8_t i = 0x0; i < 0x10; i++)
	{
		stack[i] = 0x0;
	}
}

void Chip8::clearScreen()
{
	for(uint16_t i = 0; i < 0x40 * 0x20; i++)
	{
		screen[i] = 0x0;
	}
}

void Chip8::clearKeypad()
{
	for(uint8_t i = 0x0; i < 0x10; i++)
	{
		keypad[i] = 0x0;
	}
}

Key Chip8::getKey()
{
	std::cout << "INFO: getting key\n";
	return false;
	//TODO GETCH
}

void Chip8::setKeypad()
{
	//TODO READ KEYS
}

void Chip8::beep()
{
	std::cout << "INFO: beep\n";
	//TODO
}
