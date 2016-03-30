#include "Chip8.h"
#include <iostream>
#include <map>
#include "SDL.h"
using namespace std;


bool interrupted = false;

void Chip8ErrCallback()
{
	cout << "Interrupted\n";
	interrupted = true;
}

int keyMap[256];
void createKeyMap()
{
	keyMap[SDLK_1] = 0x1;
	keyMap[SDLK_2] = 0x2;
	keyMap[SDLK_3] = 0x3;
	keyMap[SDLK_4] = 0xC;
    	keyMap[SDLK_q] = 0x4;
	keyMap[SDLK_w] = 0x5;
	keyMap[SDLK_e] = 0x6; 
	keyMap[SDLK_r] = 0xD;
    	keyMap[SDLK_a] = 0x7; 
	keyMap[SDLK_s] = 0x8;
	keyMap[SDLK_d] = 0x9; 
	keyMap[SDLK_f] = 0xE;
    	keyMap[SDLK_z] = 0xA; 
	keyMap[SDLK_x] = 0x0; 
	keyMap[SDLK_c] = 0xB;
	keyMap[SDLK_v] = 0xF;
    	keyMap[SDLK_5] = 0x5;
	keyMap[SDLK_6] = 0x6;
	keyMap[SDLK_7] = 0x7;
    	keyMap[SDLK_8] = 0x8; 
	keyMap[SDLK_9] = 0x9; 
	keyMap[SDLK_0] = 0x0; 
	keyMap[SDLK_ESCAPE] = -1;
}

int main(int argc, char** argv)
{
	Chip8 chip8;	
	chip8.Init();	
	const char* programFile = "C:/Projects/VisualStudio2010/chip8/pong2.c8";
	chip8.LoadProgram(programFile);
	//chip8.DumpMemory();	

	// SDL key to chip8 map
	createKeyMap();

	// Create Display
	SDL_Window* window = SDL_CreateWindow(argv[1], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*10, HEIGHT*10, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);	

	int temp = 0;
	unsigned lastTick = SDL_GetTicks();
	while (!interrupted)
	{
		chip8.Exec(Chip8ErrCallback);	
		
		for (SDL_Event e; SDL_PollEvent(&e); )
		{
			if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
			{			
				chip8.KeySig(keyMap[e.key.keysym.sym], e.type == SDL_KEYDOWN);
			}
		}

		// Render graphics
		Uint32 pixels[WIDTH*HEIGHT];
		chip8.RenderTo(pixels);
        SDL_UpdateTexture(texture, nullptr, pixels, 4*WIDTH);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

		if (SDL_GetTicks() - lastTick >= 16)
		{
			chip8.SigDelayTimer();
		}
	}
	
	SDL_Delay(500);
	SDL_Quit();

	system("pause");
	return 0;
}
