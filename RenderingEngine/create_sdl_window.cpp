//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>

#include <cassert>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	//Initialize SDL
	assert(SDL_Init(SDL_INIT_VIDEO) > -1);

	//Create window
	window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	assert(window != NULL);

	//Get window surface
	screenSurface = SDL_GetWindowSurface(window);

	SDL_Event e; 
	bool quit = false; 

	while (quit == false) 
	{
		//Fill the surface white
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

		//Update the surface
		SDL_UpdateWindowSurface(window);

		while (SDL_PollEvent(&e)) 
		{ 
			if (e.type == SDL_QUIT) 
				quit = true; 
		} 
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
