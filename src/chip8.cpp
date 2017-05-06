#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
//
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
//
#include "chip8.hpp"

Chip8::Chip8() :
	emulating(false),
	addressRegister(0x0),
	programCounter(0x200),
	stackPointer(0x0),
	delayTimer(0x0),
	soundTimer(0x0),
	window(sf::VideoMode(0x40 * pixelSize, 0x20 * pixelSize), "Chip-8 Emulator")
{
	srand(time(NULL));
	clearMemory();
	clearRegisters();
	clearStack();
	clearScreen();
	clearKeypad();
	loadFontSet();
	loadSound();
	std::cout << "INFO: created Chip8 object\n";
}

void Chip8::load(std::string const &name)
{
	std::ifstream file;
	file.open(name, std::ios::binary | std::ios::ate);
	if(file.is_open())
	{
		std::streampos size = file.tellg();
		char *memoryBlock = new char [size];
		file.seekg(0, std::ios::beg);
		file.read(memoryBlock, size);
		file.close();
		for(uint16_t i = 0x0; i < size && i < 0x1000 - 0x200; i++)
		{
			memory[i + 0x200] = memoryBlock[i];
		}
		delete[] memoryBlock;
		std::cout << "INFO: loaded file "
				  << name << " with size of "
				  << size << "\n";
		return;
	}
	std::cout << "ERROR: couldn't open file " << name << "\n";
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
		renderWindow();
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
	clearKeypad();
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
			std::cout << "ERROR: invalid opcode "
					  << std::hex << opcode << "\n";
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
		
		default:
			std::cout << "ERROR: invalid opcode "
					  << std::hex << opcode << "\n";
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
	{
		uint16_t positionX = registers[x];
		uint16_t positionY = registers[y];
		uint8_t height = opcode & 0x000F;
		
		registers[0xF] = 0;
		for(uint16_t iY = 0x0; iY < height; iY++)
		{
			Byte pixel = memory[addressRegister + iY];
			for(uint16_t iX = 0x0; iX < 0x8; iX++)
			{
				if((pixel & (0x80 >> iX)) != 0x0)
				{
					if(screen[(iX + positionX) +
							 ((iY + positionY) * 0x40)] == 1)
					{
						registers[0xF] = 0x1;
					}
					screen[(iX + positionX) +
							 ((iY + positionY) * 0x40)] ^= 0x1;
				}
			}
		}
		break;
	}
	
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
		
		default:
			std::cout << "ERROR: invalid opcode "
					  << std::hex << opcode << "\n";
			break;
		}
		break;
		
	default:
		std::cout << "ERROR: invalid opcode "
				  << std::hex << opcode << "\n";
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

void Chip8::loadSound()
{
	if(soundBuffer.loadFromFile("beep.wav"))
	{
		std::cout << "INFO: loaded beep.wav\n";
	}
	else
	{
		std::cout << "ERROR: couldn't load beep.wav\n";
	}
}

void Chip8::clearMemory()
{
	std::cout << "INFO: clearing memory\n";
	for(uint16_t i = 0x0; i < 0x1000; i++)
	{
		memory[i] = 0x0;
	}
}

void Chip8::clearRegisters()
{
	std::cout << "INFO: clearing registers\n";
	for(uint8_t i = 0x0; i < 0x10; i++)
	{
		registers[i] = 0x0;
	}
}

void Chip8::clearStack()
{
	std::cout << "INFO: clearing stack\n";
	for(uint8_t i = 0x0; i < 0x10; i++)
	{
		stack[i] = 0x0;
	}
}

void Chip8::clearScreen()
{
	std::cout << "INFO: clearing screen\n";
	for(uint16_t i = 0; i < 0x40 * 0x20; i++)
	{
		screen[i] = 0x0;
	}
}

void Chip8::clearKeypad()
{
	std::cout << "INFO: clearing keypad\n";
	for(uint8_t i = 0x0; i < 0x10; i++)
	{
		keypad[i] = 0x0;
	}
}

Key Chip8::getKey()
{
	std::cout << "INFO: getting key\n";
	sf::Event keyEvent;
	while(true)
	{
		window.pollEvent(keyEvent);
		if(keyEvent.type == sf::Event::KeyPressed)
		{
			if(keyEvent.key.code == sf::Keyboard::X)
			{
				return 0x0;
			}
			if(keyEvent.key.code == sf::Keyboard::Num1)
			{
				return 0x1;
			}
			if(keyEvent.key.code == sf::Keyboard::Num2)
			{
				return 0x2;
			}
			if(keyEvent.key.code == sf::Keyboard::Num3)
			{
				return 0x3;
			}
			if(keyEvent.key.code == sf::Keyboard::Q)
			{
				return 0x4;
			}
			if(keyEvent.key.code == sf::Keyboard::W)
			{
				return 0x5;
			}
			if(keyEvent.key.code == sf::Keyboard::E)
			{
				return 0x6;
			}
			if(keyEvent.key.code == sf::Keyboard::A)
			{
				return 0x7;
			}
			if(keyEvent.key.code == sf::Keyboard::S)
			{
				return 0x8;
			}
			if(keyEvent.key.code == sf::Keyboard::D)
			{
				return 0x9;
			}
			if(keyEvent.key.code == sf::Keyboard::Z)
			{
				return 0xA;
			}
			if(keyEvent.key.code == sf::Keyboard::C)
			{
				return 0xB;
			}
			if(keyEvent.key.code == sf::Keyboard::Num4)
			{
				return 0xC;
			}
			if(keyEvent.key.code == sf::Keyboard::R)
			{
				return 0xD;
			}
			if(keyEvent.key.code == sf::Keyboard::F)
			{
				return 0xE;
			}
			if(keyEvent.key.code == sf::Keyboard::V)
			{
				return 0xF;
			}
		}
	}
}

void Chip8::setKeypad()
{
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::X))
	{
		keypad[0x0] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
	{
		keypad[0x1] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
	{
		keypad[0x2] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
	{
		keypad[0x3] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		keypad[0x4] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		keypad[0x5] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		keypad[0x6] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		keypad[0x7] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		keypad[0x8] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		keypad[0x9] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
	{
		keypad[0xA] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::C))
	{
		keypad[0xB] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
	{
		keypad[0xC] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		keypad[0xD] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::F))
	{
		keypad[0xE] = true;
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::V))
	{
		keypad[0xF] = true;
	}
}

void Chip8::beep()
{
	std::cout << "INFO: beep\n";
	sf::Sound beep;
	beep.setBuffer(soundBuffer);
	beep.play();
}

void Chip8::renderWindow()
{
	window.clear(sf::Color::Black);
	sf::RectangleShape rectangle(sf::Vector2f(pixelSize, pixelSize));
	for(uint16_t iX = 0x0; iX < 0x40; iX++ )
	{
		for(uint16_t iY = 0x0; iY < 0x20; iY++ )
		{
			if(screen[iX + (iY * 0x40)])
			{
				rectangle.setPosition( iX * pixelSize, iY * pixelSize );
				window.draw(rectangle);
			}
		}
	}
	window.display();
}
