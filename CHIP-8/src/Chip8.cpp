#include "Chip8.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

unsigned const char Chip8::FontSet[16*5] = 
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

void Chip8::Init()
{
	// Load Initial values
	I = 0x0;
	PC = 0x200;
	SP = 0;

	DelayTimer = 0;
	SoundTimer = 0;

	ReadFontSet();

	Clrscr();

	ClrKeys();
}

void Chip8::ClrKeys()
{
	for (int i = 0 ; i < 16; i++)
	{
		Keys[i] = 0x0;
	}
}

void Chip8::Clrscr()
{
	for (int i = 0 ; i < WIDTH*HEIGHT ; i++)
	{
		DispMem[i] = 0x0;
	}
}

void Chip8::KeySig(unsigned char key, bool pressed)
{
	Keys[key] = pressed;
	//printf("Chip8 reviced key signal Keys[%d] = %d\n",key,pressed);
}

void Chip8::SigDelayTimer()
{
	--DelayTimer;
}

void Chip8::Reset()
{
	// Clear memory

	// Clear registers
	this->Init();
}

void Chip8::ReadFontSet()
{
	for (int i = 0 ; i < FONTSET_SIZE ; i++)
	{
		Memory[0x50 + i] = FontSet[i];
	}
}

void Chip8::LoadProgram(const char* file)
{
	cout << "Loading " << file << " ...\n";      
	int offset = 0;
    	for (std::ifstream f(file, std::ios::binary); f.good();)
	{
		Memory[0x200 + (offset++)] = f.get();                        
	}
	cout << "Loaded\n";
}

void Chip8::DumpMemory()
{
	cout << "---------------- Memory Dump ------------------\n";
	for (int i = 0 ; i < MEMORY_SIZE ; i++)
	{
		printf("%x ",Memory[i]);
		if (i % 32 == 0 && i!= 0)
			cout << "\n";
	}
	cout << "\n------------------------------------------------\n";
}

// Render the screen
 void Chip8::RenderTo(Uint32* pixels)
{
	for(unsigned pos = 0 ; pos < WIDTH*HEIGHT ; ++pos)
	{
		if (DispMem[pos] == 1)
			pixels[pos] = 0xFFFFFF;
		else
			pixels[pos] = 0x0;		
	}
}

void Chip8::Exec(void(*ErrCallBack)())
{
	// Fetch opcode
	unsigned opcode =  (Memory[PC] << 8 ) | Memory[PC +1];	
	PC += 2;

	// Extract opcode
	unsigned INS = opcode >> 12;	
	unsigned X   = (opcode & 0x0F00) >> 8 ;
	unsigned Y   = (opcode & 0x00F0) >> 4;
	unsigned N   = opcode & 0x000F;
	unsigned NN  = opcode & 0x00FF;
	unsigned NNN = opcode & 0x0FFF;

	unsigned char &VX = V[X], &VY = V[Y] , &VF = V[0xF];

	printf("opcode : %X   ", opcode);
	switch(INS)
	{
		case 0x0:
		{
			switch(NN)
			{
				//Ignored :-  0NNN Calls RCA 1802 program at address NNN. Not necessary for most ROMs.

				case 0xE0: // 00E0 Clears the screen.
				{
					break;
					ErrCallBack();
				}
				case 0xEE:// 00EE Returns from a subroutine.
				{
					PC = Stack[--SP];
					printf("\tReturn to %X\n",PC);
					break;
				}
			}
			break;
		}

		case 0x1: //  1NNN 	Jumps to address NNN.
		{
			PC = NNN;
			printf("\tJump to address %X\n",NNN);
			break;
		}
		
		case 0x2:	// 2NNN Calls subroutine at NNN.
		{
			Stack[SP++] = PC;
			PC = NNN;
			printf("\tJump to %X  from %x\n", NNN, Stack[SP-1]);
			break;
		}
		case 0x3:	//  3XNN 	Skips the next instruction if VX equals NN.
		{
			if (VX == NN)
				PC += 2;			
			printf("\tSkip next ins if VX == NN : (%X == %X)\n",VX,NN);
			break;
		}
		case 0x4: //  4XNN 	Skips the next instruction if VX doesn't equal NN.
		{
			if (VX != NN)
				PC += 2;
			printf("\tSkip next instr if V[%X] != NN    %X != %X\n",X,VX,NN);
			break;
		}
		case 0x5: //  5XY0 	Skips the next instruction if VX equals VY.
		{
			if (VX == NN)
				PC += 2;
			printf("\tSkip next instr if V[%X] == NN    %X == %X\n",X,VX,NN);
			break;
		}
		case 0x6:	//  6XNN 	Sets VX to NN.
		{
			VX = NN;
			printf("\tSet V[%X] = %X\n", X , NN);
			break;
		}
		case 0x7:	//  7XNN 	Adds NN to VX.
		{
			VX += NN;
			printf("\tAdd VX += %X\n",NN);
			break;
		}
		case 0x8:
		{
			switch (N)
			{
				case 0x0://  8XY0 	Sets VX to the value of VY.
				{
					VX = VY;
					printf("\tSet V[%X] = V[%X]\n",X,Y);
					break;
				}
				case 0x1: //  8XY1 	Sets VX to VX or VY.
				{
					VX |= VY;
					printf("\tSet V[%X] |= V[%X]\n",X,Y);
					break;
				}
				case 0x2: //  8XY2 	Sets VX to VX and VY.
				{
					VX &= VY;
					printf("\tSet V[%X] &= V[%X]\n",X,Y);
					break;
				}
				case 0x3: //  8XY3 	Sets VX to VX xor VY.
				{
					VX ^= VY;
					printf("\tSet V[%X] ^= V[%X]\n",X,Y);
					break;
				}
				case 0x4: //  8XY4 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
				{
					unsigned Temp = VX + VY;
					VX = Temp;
					VF = Temp >> 8;
					printf("\tSet V[%X] += V[%X]  , VF=%X\n",X,Y,VF);
					break;
				}

				case 0x5: //  8XY5 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				{
					unsigned Temp = VX - VY;
					VX = !(Temp >> 8);
					VX = Temp;
					printf("\tSet V[%X] = V[%X] - V[%X]  = %X - %X    ,   VF = %X\n",X,X,Y,VX,VY,VF);
					break;
				}
				case 0x6: //  8XY6 	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
				{
					VF = VX & 0X1;
					VX = VX >> 1;
					printf("\tShift V[%X] >> 1   , VF = %X\n",X,VF);
					break;
				}
				case 0x7:  //  8XY7 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				{
					unsigned Temp = VY - VX;
					VX = !(Temp >> 8);
					VX = Temp;
					printf("\tSet V[%X] = V[%X] - V[%X]  = %X - %X    ,   VF = %X\n",X,Y,X,VY,VX,VF);
					break;
				}
				case 0xE: //  8XYE 	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
				{
					VX = VX >> 7;
					VF = VX << 1;
					printf("\tShift V[%X] << 1   , VF = %X\n",X,VF);
					break;
				}
				default:
				{
					ErrCallBack();
				}
			}
			break;
		}
		case 0x9: //  9XY0 	Skips the next instruction if VX doesn't equal VY.
		{
			if (VX != VY)
				PC += 2;
			printf("\tSkip next instr if V[%X] != V[%Y]    %X == %X\n",X,Y,VX,VY);
			break;
		}
		case 0xA://  ANNN 	Sets I to the address NNN.
		{
			I = NNN;
			printf("\tSet I = %X\n",NNN);
			break;
		}
		case 0xB:  //  BNNN 	Jumps to the address NNN plus V0.
		{
			PC = NNN + V[0];
			printf("\tJump to addr %X + %X\n",NNN,V[0]);
			break;
		}
		case 0xC:	//  CXNN 	Sets VX to the result of a bitwise and operation on a random number and NN.
		{
			srand(time(NULL));
			std::cout<<std::rand()<< endl;
			unsigned rand = (std::rand() ) && 0xFF;
			VX = (rand | NN);
			printf("\tSet V[%X] = %X | %X\n",X,rand,NN);
			break;
		}
		case 0xD: /*   DXYN Sprites stored in memory at location in index register (I), 8bits wide.
					   Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. 
					   All drawing is XOR drawing (i.e. it toggles the screen pixels).
					   Sprites are drawn starting at position VX, VY. N is the number of 8bit rows that need to be drawn. 
					   If N is greater than 1, second line continues at position VX, VY+1, and so on. */
		{
			VF = 0;
			for (unsigned y = 0 ; y < N ; y++)
			{
				unsigned char line = Memory[I + y];
				for (unsigned x = 0 ; x < 8 ; x++)
				{
					unsigned pixel = line & (0x80 >> x);
					if (pixel != 0)
					{
						unsigned index = (VY + y) * 64 + (VX + x);
						if (DispMem[index] == 1)
							VF = 1;
						DispMem[index] ^= 1;
					}
				}
			}
			printf("\tSprite draw @ V[%X],V[%X]\n", X,Y);
			break;
		}
		case 0xE: 
		{
			switch (NN) 
			{
				case 0x9E:  //  EX9E 	Skips the next instruction if the key stored in VX is pressed.
				{
					if (Keys[VX] == 1)
						PC += 2;
					break;
				}
				case 0xA1://  EXA1 	Skips the next instruction if the key stored in VX isn't pressed.
				{
					if (Keys[VX] != 0)
						PC += 2;
					break;
				}
			}
			break;
		}
		case 0xF:
		{
			switch (NN)
			{
				case 0x07: //  FX07 	Sets VX to the value of the delay timer.
				{
					VX = DelayTimer;
					printf("\tSet V[%X]=%X\n",X,VX);
					break;
				}
				//  FX0A 	A key press is awaited, and then stored in VX.
				case 0x15://  FX15 	Sets the delay timer to VX.
				{
					DelayTimer = VX;
					printf("\tSet DelayTimer=%X\n",DelayTimer);
					break;
				}
				case 0x18://  FX18 	Sets the sound timer to VX.
				{
					SoundTimer = VX;
					printf("\tSet SoundTimer = V[%X]    %X\n",X,VX);
					break;
				}
				//  FX1E 	Adds VX to I			
				case 0x29://  FX29 	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				{
					I = 0x50 + VX*5;
					printf("\tSet I to charater in %X\n",VX);
					break;
				}
				case 0x33: /*  FX33 	Stores the Binary-coded decimal representation of VX, 
						   with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. 
						   (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, 
									the tens digit at location I+1, and the ones digit at location I+2.)*/
				{
					Memory[I]     = (VX/100) % 10;
					Memory[I + 1] = (VX/10)  % 10;
					Memory[I + 2] = (VX) % 10;
					printf("\tStore BCD of V[%X]  @Mem[%d]  as (%d, %d, %d)\n",X, I,Memory[I],Memory[I+1],Memory[I+2]);
					break;
				}
				//  FX55 	Stores V0 to VX (including VX) in memory starting at address I.
				case 0x65:	//  FX65 	Fills V0 to VX (including VX) with values from memory starting at address I.
				{
					for (unsigned i = 0 ; i <= X ; i++)
					{
						V[i] = Memory[I+i];
					}
					printf("\tFill V[0] to V[%d] starting from %X\b",X,I);
					break;
				}
				default:
				{
					printf("\tInvalid Instruction\n");
					ErrCallBack();
					break;
				}
			}
			break;
		}
		default:
		{
			printf("\tInvalid Instruction\n");
			ErrCallBack();
			break;
		}
	}
}
