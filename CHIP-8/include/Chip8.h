#pragma once

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

#include <string>
#include "defines.h"
#include <stdint.h>
#include "SDL.h"

using namespace std;
class Chip8
{
	public:
		void Init();
		void Reset();
		void Clrscr();
		void ClrKeys();
		void KeySig(unsigned char key,bool pressed);
		void SigDelayTimer();
		void ReadFontSet();
		void LoadProgram(const char*);
		void DumpMemory();
		void RenderTo(Uint32 * pixels);

		void Exec(void(*ErrCallback)());

	private:
		// 4KB of memory
		unsigned char Memory[MEMORY_SIZE];

		// 16 registers
		unsigned char V[16];
		unsigned short I;

		unsigned short PC;
		unsigned short Stack[12];
		unsigned short SP;

		unsigned char Keys[16];
		unsigned char DelayTimer;
		unsigned char SoundTimer;
		unsigned char WaitingKey;

		// Display
        	unsigned char DispMem[64*32];
		static const unsigned char FontSet[FONTSET_SIZE];// monoc char bitmap
};
