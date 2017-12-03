#include "desktopController.hpp"
#include <cmath>

DesktopController::DesktopController(){

}
DesktopController::~DesktopController(){

}

bool DesktopController::initialize(size_t width, size_t height,
                          std::shared_ptr<BaseInput> input, bool debug){
   BaseController::initialize(width, height, input, debug);
   m_upscale = 30;
   m_upscaleWidth = width*m_upscale;
   m_upscaleHeight= height*m_upscale;

   if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0)
   {
  	std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
  	SDL_Quit();
  	return false;
   }
   m_window = SDL_CreateWindow("LED Table",
                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                      m_upscaleWidth, m_upscaleHeight,
                      SDL_WINDOW_SHOWN);
   if(!m_window)
   {
      // Fehler! Fehlermeldung ausgeben und Programm beenden.
      std::cerr << "Konnte SDL-Fenster nicht erzeugen! Fehler: " << SDL_GetError() << std::endl;
      return false;
   }
   m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
   if(!m_renderer)
   {
     // Fehler! Fehlermeldung ausgeben und Programm beenden.
     std::cerr << "Konnte SDL-Renderer nicht erzeugen! Fehler: " << SDL_GetError() << std::endl;
     return false;
   }

   Uint32 rmask, gmask, bmask, amask;
   /* SDL interprets each pixel as a 32-bit number, so our masks must depend
   on the endianness (byte order) of the machine */
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif

   m_imageSurf = SDL_CreateRGBSurface(0, width, height, 32 , rmask, gmask, bmask, amask);
   m_windowSurf = SDL_GetWindowSurface(m_window);

   return true;

}
void DesktopController::copyImageToBuffer(const std::vector<uint8_t>&frame){
   SDL_LockSurface(m_imageSurf);

   int rowstride = m_imageSurf->pitch;
   unsigned char* pixels = (unsigned char*)m_imageSurf->pixels;
   const BaseApplication::Palette& palette = getCurrentPalette();
   size_t idx = 0;
   uint8_t r,g,b;
   for (size_t y = 0; y < m_height; y++) {
     unsigned char* p = pixels + y*rowstride;
     for (size_t x = 0; x < m_width; x++) {

       int c = frame[idx++];
       r = palette[c*3];
       g = palette[c*3 + 1];
       b = palette[c*3 + 2];
       *p++ = r;
       *p++ = g;
       *p++ = b;
       *p++ = 255;
     }
   }
   SDL_UnlockSurface(m_imageSurf);
}

void DesktopController::showFrame(const std::vector<uint8_t>&frame){
  // Wir holen uns so lange neue Ereignisse, bis es keine mehr gibt.
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
      // Was ist passiert?
      switch(event.type)
      {
      case SDL_QUIT:
          m_isRunning = false;
          break;
      }
  }

  SDL_RenderClear(m_renderer);
  copyImageToBuffer(frame);
  SDL_Texture* bitmapTex = SDL_CreateTextureFromSurface(m_renderer, m_imageSurf);
  SDL_RenderCopy(m_renderer, bitmapTex, NULL, NULL);
  SDL_RenderPresent(m_renderer);
  SDL_DestroyTexture(bitmapTex);
}

void DesktopController::shutdown(){
  SDL_FreeSurface(m_imageSurf);
  SDL_Quit();
}
