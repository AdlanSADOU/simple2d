// simple2d.c

#include "../include/simple2d.h"

// Flag for printing diagnostic messages
static bool diagnostics = false;

// Initalize S2D shared data
char S2D_msg[1024];

// S2D initialization status
static bool initted = false;


/*
 * Checks if a file exists and can be accessed
 */
static bool file_exists(const char *path) {
  if (access(path, F_OK) != -1) {
    return true;
  } else {
    return false;
  }
}


/*
 * Logs standard messages to the console
 */
void S2D_Log(const char *msg, int type) {
  if (diagnostics) {
    switch (type) {
      case S2D_INFO:
        printf("\033[4;36mInfo:\033[0m %s\n", msg);
        break;
      case S2D_WARN:
        printf("\033[4;33mWarning:\033[0m %s\n", msg);
        break;
      case S2D_ERROR:
        printf("\033[4;31mError:\033[0m %s\n", msg);
        break;
      default:
        printf("%s\n", msg);
    }
  }
}


/*
 * Logs Simple 2D errors to the console, with caller and message body
 */
void S2D_Error(const char *caller, const char *msg) {
  sprintf(S2D_msg, "(%s) %s", caller, msg);
  S2D_Log(S2D_msg, S2D_ERROR);
}


/*
 * Enable/disable logging of diagnostics
 * Initialize Simple 2D subsystems
 */
void S2D_Diagnostics(bool status) {
  diagnostics = status;
static bool S2D_Init() {
  if (initted) return true;
  
  S2D_Log("Initializing Simple 2D", S2D_INFO);
  
  // Initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    S2D_Error("SDL_Init", SDL_GetError());
    return false;
  }
  
  // Initialize SDL_ttf
  if (TTF_Init() != 0) {
    S2D_Error("TTF_Init", TTF_GetError());
    return false;
  }
  
  // Initialize SDL_mixer
  int mix_flags = MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_MP3;
  int mix_initted = Mix_Init(mix_flags);
  if ((mix_initted&mix_flags) != mix_flags) {
    S2D_Error("Mix_Init", Mix_GetError());
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
    S2D_Error("Mix_OpenAudio", Mix_GetError());
    return false;
  }
  
  // All subsystems initted
  initted = true;
  return true;
}


/*
 * Quits S2D subsystems
 */
static void S2D_Quit() {
  IMG_Quit();
  Mix_CloseAudio();
  Mix_Quit();
  TTF_Quit();
  SDL_Quit();
  initted = false;
}


/*
 * Draw a triangle
 */
void S2D_DrawTriangle(GLfloat x1,  GLfloat y1,
                      GLfloat c1r, GLfloat c1g, GLfloat c1b, GLfloat c1a,
                      GLfloat x2,  GLfloat y2,
                      GLfloat c2r, GLfloat c2g, GLfloat c2b, GLfloat c2a,
                      GLfloat x3,  GLfloat y3,
                      GLfloat c3r, GLfloat c3g, GLfloat c3b, GLfloat c3a) {
  
  S2D_GL_DrawTriangle(x1, y1, c1r, c1g, c1b, c1a,
                      x2, y2, c2r, c2g, c2b, c2a,
                      x3, y3, c3r, c3g, c3b, c3a);
}


/*
 * Draw a quad, using two triangles
 */
void S2D_DrawQuad(GLfloat x1,  GLfloat y1,
                  GLfloat c1r, GLfloat c1g, GLfloat c1b, GLfloat c1a,
                  GLfloat x2,  GLfloat y2,
                  GLfloat c2r, GLfloat c2g, GLfloat c2b, GLfloat c2a,
                  GLfloat x3,  GLfloat y3,
                  GLfloat c3r, GLfloat c3g, GLfloat c3b, GLfloat c3a,
                  GLfloat x4,  GLfloat y4,
                  GLfloat c4r, GLfloat c4g, GLfloat c4b, GLfloat c4a) {
  
  S2D_GL_DrawTriangle(x1, y1, c1r, c1g, c1b, c1a,
                      x2, y2, c2r, c2g, c2b, c2a,
                      x3, y3, c3r, c3g, c3b, c3a);
  
  S2D_GL_DrawTriangle(x3, y3, c3r, c3g, c3b, c3a,
                      x4, y4, c4r, c4g, c4b, c4a,
                      x1, y1, c1r, c1g, c1b, c1a);
};


/*
 * Create an image
 * Params: path = image file path
 */
S2D_Image *S2D_CreateImage(const char *path) {
  S2D_Init();
  
  // Check if image file exists
  if (!file_exists(path)) {
    S2D_Error("S2D_CreateImage", "Image file not found");
    return NULL;
  }
  
  // Load image from file as SDL_Surface
  SDL_Surface *surface = IMG_Load(path);
  if (!surface) {
    S2D_Error("IMG_Load", IMG_GetError());
    return NULL;
  }
  
  // Allocate the image structure
  S2D_Image *img = (S2D_Image *) malloc(sizeof(S2D_Image));
  if(!img) {
    S2D_Error("IMG_Load", "Out of memory!");
    SDL_FreeSurface(surface);
    return NULL;
  }
  
  // Initialize values
  img->color.r = 1.f;
  img->color.g = 1.f;
  img->color.b = 1.f;
  img->color.a = 1.f;
  img->x = 0;
  img->y = 0;
  img->w = surface->w;
  img->h = surface->h;
  img->texture_id = 0;
  
  // Detect image mode
  int format = GL_RGB;
  if(surface->format->BytesPerPixel == 4) {
    format = GL_RGBA;
  }
  
  // Flip image bits if BGA
  
  Uint32 r = surface->format->Rmask;
  Uint32 g = surface->format->Gmask;
  Uint32 a = surface->format->Amask;
  
  if (r&0xFF000000 || r&0xFF0000) {
    char *p = (char *)surface->pixels;
    int bpp = surface->format->BytesPerPixel;
    int w = surface->w;
    int h = surface->h;
    char tmp;
    for (int i = 0; i < bpp * w * h; i += bpp) {
      if (a&0xFF) {
        tmp = p[i];
        p[i] = p[i+3];
        p[i+3] = tmp;
      }
      if (g&0xFF0000) {
        tmp = p[i+1];
        p[i+1] = p[i+2];
        p[i+2] = tmp;
      }
      if (r&0xFF0000) {
        tmp = p[i];
        p[i] = p[i+2];
        p[i+2] = tmp;
      }
    }
  }
  
  // Set up the texture for rendering
  S2D_GL_SetUpTexture(&img->texture_id, format, img->w, img->h, surface->pixels, GL_NEAREST);
  
  // Free the surface data, no longer needed
  SDL_FreeSurface(surface);
  
  return img;
}


/*
 * Draw an image
 */
void S2D_DrawImage(S2D_Image *img) {
  if (!img) return;
  S2D_GL_DrawImage(img);
}


/*
 * Free an image
 */
void S2D_FreeImage(S2D_Image *img) {
  if (!img) return;
  S2D_GL_FreeTexture(&img->texture_id);
  free(img);
}


/*
 * Create text
 * Params: font = font file path; msg = text to display; size = font size
 * Returns NULL if text could not be created
 */
S2D_Text *S2D_CreateText(const char *font, const char *msg, int size) {
  S2D_Init();
  
  // Check if font file exists
  if (!file_exists(font)) {
    S2D_Error("S2D_CreateText", "Font file not found");
    return NULL;
  }
  
  // `msg` cannot be an empty string or NULL for TTF_SizeText
  if (msg == NULL || strlen(msg) == 0) msg = " ";
  
  // Allocate the text structure
  S2D_Text *txt = (S2D_Text *) malloc(sizeof(S2D_Text));
  if (!txt) {
    S2D_Error("S2D_CreateText", "Out of memory!");
    return NULL;
  }
  
  // Set default values
  txt->msg = msg;
  txt->x = 0;
  txt->y = 0;
  txt->color.r = 1.0;
  txt->color.g = 1.0;
  txt->color.b = 1.0;
  txt->color.a = 1.0;
  txt->texture_id = 0;
  
  // Open the font
  txt->font = TTF_OpenFont(font, size);
  if (!txt->font) {
    S2D_Error("TTF_OpenFont", TTF_GetError());
    free(txt);
    return NULL;
  }
  
  // Save the width and height of the text
  TTF_SizeText(txt->font, txt->msg, &txt->w, &txt->h);
  
  // Assign color and set up for rendering
  SDL_Color color = { 255, 255, 255 };
  SDL_Surface *surface = TTF_RenderText_Blended(txt->font, txt->msg, color);
  S2D_GL_SetUpTexture(&txt->texture_id, GL_RGBA, txt->w, txt->h, surface->pixels, GL_NEAREST);
  
  // Free the surface data, no longer needed
  SDL_FreeSurface(surface);
  
  return txt;
}


/*
 * Sets the text message
 */
void S2D_SetText(S2D_Text *txt, const char *msg) {
  if (!txt) return;
  
  // `msg` cannot be an empty string or NULL for TTF_SizeText
  if (msg == NULL || strlen(msg) == 0) msg = " ";
  
  txt->msg = msg;
  
  TTF_SizeText(txt->font, txt->msg, &txt->w, &txt->h);
  
  SDL_Color color = { 255, 255, 255 };
  SDL_Surface *surface = TTF_RenderText_Blended(txt->font, txt->msg, color);
  
  S2D_GL_SetUpTexture(&txt->texture_id, GL_RGBA, txt->w, txt->h, surface->pixels, GL_NEAREST);
  
  SDL_FreeSurface(surface);
}


/*
 * Draw text
 */
void S2D_DrawText(S2D_Text *txt) {
  if (!txt) return;
  S2D_GL_DrawText(txt);
}


/*
 * Free the text
 */
void S2D_FreeText(S2D_Text *txt) {
  if (!txt) return;
  S2D_GL_FreeTexture(&txt->texture_id);
  TTF_CloseFont(txt->font);
}


/*
 * Create a sound
 */
S2D_Sound *S2D_CreateSound(const char *path) {
  S2D_Init();
  
  // Check if sound file exists
  if (!file_exists(path)) {
    S2D_Error("S2D_CreateSound", "Sound file not found");
    return NULL;
  }
  
  // Allocate the sound structure
  S2D_Sound *sound = (S2D_Sound *) malloc(sizeof(S2D_Sound));
  
  // Load the sound data from file
  sound->data = Mix_LoadWAV(path);
  if (!sound->data) {
    S2D_Error("Mix_LoadWAV", Mix_GetError());
    free(sound);
    return NULL;
  }
  
  return sound;
}


/*
 * Play the sound
 */
void S2D_PlaySound(S2D_Sound *sound) {
  if (!sound) return;
  Mix_PlayChannel(-1, sound->data, 0);
}


/*
 * Free the sound
 */
void S2D_FreeSound(S2D_Sound *sound) {
  if (!sound) return;
  Mix_FreeChunk(sound->data);
  free(sound);
}


/*
 * Create the music
 */
S2D_Music *S2D_CreateMusic(const char *path) {
  S2D_Init();
  
  // Check if music file exists
  if (!file_exists(path)) {
    S2D_Error("S2D_CreateMusic", "Music file not found");
    return NULL;
  }
  
  // Allocate the music structure
  S2D_Music *music = (S2D_Music *) malloc(sizeof(S2D_Music));
  if(!music) {
    S2D_Error("S2D_CreateMusic", "Out of memory!");
    return NULL;
  }
  
  // Load the music data from file
  music->data = Mix_LoadMUS(path);
  if (!music->data) {
    S2D_Error("Mix_LoadMUS", Mix_GetError());
    free(music);
    return NULL;
  }
  
  return music;
}


/*
 * Play the music
 */
void S2D_PlayMusic(S2D_Music *music, int times) {
  if (!music) return;
  
  // times: 0 == once, -1 == forever
  if (Mix_PlayMusic(music->data, times) == -1) {
    // No music for you
    S2D_Error("S2D_PlayMusic", Mix_GetError());
  }
}


/*
 * Pause the playing music
 */
void S2D_PauseMusic() {
  Mix_PauseMusic();
}


/*
 * Resume the current music
 */
void S2D_ResumeMusic() {
  Mix_ResumeMusic();
}


/*
 * Stops the playing music; interrupts fader effects
 */
void S2D_StopMusic() {
  Mix_HaltMusic();
}


/*
 * Fade out the playing music
 */
void S2D_FadeOutMusic(int ms) {
  Mix_FadeOutMusic(ms);
}


/*
 * Free the music
 */
void S2D_FreeMusic(S2D_Music *music) {
  if (!music) return;
  Mix_FreeMusic(music->data);
  free(music);
}


/*
 * Create a window
 */
S2D_Window *S2D_CreateWindow(const char *title, int width, int height,
                             S2D_Update update, S2D_Render render, int flags) {
  
  S2D_Init();
  
  // Allocate window and set default values
  S2D_Window *window    = (S2D_Window *) malloc(sizeof(S2D_Window));
  window->close         = true;
  window->title         = title;
  window->orig_width    = width;
  window->orig_height   = height;
  window->width         = width;
  window->height        = height;
  window->viewport      = S2D_SCALE;
  window->fps_cap       = 60;
  window->vsync         = true;
  window->update        = update;
  window->render        = render;
  window->on_key        = NULL;
  window->on_key_down   = NULL;
  window->on_mouse      = NULL;
  window->on_controller = NULL;
  window->background.r  = 0.0;
  window->background.g  = 0.0;
  window->background.b  = 0.0;
  window->background.a  = 1.0;
  
  return window;
}


/*
 * Show the window
 */
int S2D_Show(S2D_Window *window) {
  if (!window) {
    S2D_Error("S2D_Show", "Window cannot be shown");
    return -1;
  }
  
  int mouse_x, mouse_y;
  const Uint8 *key_state;
  
  Uint32 frames = 0;           // Total frames since start
  Uint32 start_ms = SDL_GetTicks();  // Elapsed time since start
  Uint32 begin_ms = start_ms;  // Time at beginning of loop
  Uint32 end_ms;               // Time at end of loop
  Uint32 elapsed_ms;           // Total elapsed time
  Uint32 loop_ms;              // Elapsed time of loop
  int delay_ms;                // Amount of delay to achieve desired frame rate
  double fps;                  // The actual frame rate
  
  // Enable VSync
  if (window->vsync) {
    if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1")) {
      S2D_Log("VSync cannot be enabled", S2D_WARN);
    }
  }
  
  // Detect Controllers and Joysticks //////////////////////////////////////////
  
  if (SDL_NumJoysticks() > 0) {
    sprintf(S2D_msg, "Joysticks detected: %i", SDL_NumJoysticks());
    S2D_Log(S2D_msg, S2D_INFO);
  }
  
  // Variables for controllers and joysticks
  SDL_GameController *controller = NULL;
  SDL_Joystick *joy = NULL;
  
  // Enumerate joysticks
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    
    // Check to see if joystick supports SDL's game controller interface
    if (SDL_IsGameController(i)) {
      controller = SDL_GameControllerOpen(i);
      if (controller) {
        sprintf(S2D_msg, "Found a valid controller, named: %s\n",
                SDL_GameControllerName(controller));
        S2D_Log(S2D_msg, S2D_INFO);
        break;  // Break after first available controller
      } else {
        sprintf(S2D_msg, "Could not open game controller %i: %s\n", i, SDL_GetError());
        S2D_Log(S2D_msg, S2D_ERROR);
      }
      
    // Controller interface not supported, try to open as joystick
    } else {
      sprintf(S2D_msg, "Joystick %i is not supported by the game controller interface", i);
      S2D_Log(S2D_msg, S2D_WARN);
      joy = SDL_JoystickOpen(i);
      
      // Joystick is valid
      if (joy) {
        sprintf(S2D_msg,
          "Opened Joystick %i\n"
          "Name: %s\n"
          "Axes: %d\n"
          "Buttons: %d\n"
          "Balls: %d\n",
          i, SDL_JoystickName(joy), SDL_JoystickNumAxes(joy),
          SDL_JoystickNumButtons(joy), SDL_JoystickNumBalls(joy)
        );
        S2D_Log(S2D_msg, S2D_INFO);
        
      // Joystick not valid
      } else {
        sprintf(S2D_msg, "Could not open Joystick %i", i);
        S2D_Log(S2D_msg, S2D_ERROR);
      }
      
      break;  // Break after first available joystick
    }
  }
  
  
  window->close = false;
  
  // Create SDL window
  window->sdl = SDL_CreateWindow(
    window->title,                                   // title
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  // window position
    window->width, window->height,                   // window size
    SDL_WINDOW_OPENGL | window->flags                // flags
  );
  
  if (!window->sdl) S2D_Error("SDL_CreateWindow", SDL_GetError());
  
  // Window created by SDL might not actually be the requested size.
  // If not, retrieve and set the actual window size.
  window->s_width = window->width;
  window->s_height = window->height;
  SDL_GetWindowSize(window->sdl, &window->width, &window->height);
  
  if ((window->width != window->s_width) ||
     (window->height != window->s_height)) {
    
    sprintf(S2D_msg,
      "Resolution %dx%d unsupported by driver, scaling to %dx%d",
      window->s_width, window->s_height, window->width, window->height);
    S2D_Log(S2D_msg, S2D_WARN);
  }
  
  // Set Up OpenGL /////////////////////////////////////////////////////////////
  
  S2D_GL_Init(window);
  // Main Loop /////////////////////////////////////////////////////////////////
  
  while (!window->close) {
    
    // Clear Frame /////////////////////////////////////////////////////////////
    
    S2D_GL_Clear(window->background);
    
    // Set FPS /////////////////////////////////////////////////////////////////
    
    frames++;
    end_ms = SDL_GetTicks();
    
    elapsed_ms = end_ms - start_ms;
    fps = frames / (elapsed_ms / 1000.0);
    
    loop_ms = end_ms - begin_ms;
    delay_ms = (1000 / window->fps_cap) - loop_ms;
    
    if (delay_ms < 0) delay_ms = 0;
    
    // Note: `loop_ms + delay_ms` should equal `1000 / fps_cap`
    
    SDL_Delay(delay_ms);
    begin_ms = SDL_GetTicks();
    
    // Handle Input ////////////////////////////////////////////////////////////
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        
        case SDL_KEYDOWN:
          if (window->on_key)
            window->on_key(SDL_GetScancodeName(e.key.keysym.scancode));
          break;
        
        case SDL_MOUSEBUTTONDOWN:
          if (window->on_mouse)
            window->on_mouse(e.button.x, e.button.y);
          break;
        
        case SDL_JOYAXISMOTION:
          if (window->on_controller)
            window->on_controller(true, e.jaxis.axis, e.jaxis.value, false, 0);
          break;
        
        case SDL_JOYBUTTONDOWN:
          if (window->on_controller)
            window->on_controller(false, 0, 0, true, e.jbutton.button);
          break;
        
        case SDL_WINDOWEVENT:
          switch (e.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
              // Store new window size
              window->width  = e.window.data1;
              window->height = e.window.data2;
              
              S2D_GL_SetViewport(window);
              break;
          }
          break;
        
        case SDL_QUIT:
          S2D_Close(window);
          break;
      }
    }
    
    // Detect keys held down
    int num_keys;
    key_state = SDL_GetKeyboardState(&num_keys);
    
    for (int i = 0; i < num_keys; i++) {
      if (window->on_key_down) {
        if (key_state[i] == 1) {
          window->on_key_down(SDL_GetScancodeName(i));
        }
      }
    }
    
    // Store the mouse position
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    // Update Window State /////////////////////////////////////////////////////
    
    // Store new values in the window
    window->mouse.x      = mouse_x;
    window->mouse.y      = mouse_y;
    window->mouse.real_x = mouse_x;
    window->mouse.real_y = mouse_y;
    window->frames       = frames;
    window->elapsed_ms   = elapsed_ms;
    window->loop_ms      = loop_ms;
    window->delay_ms     = delay_ms;
    window->fps          = fps;
    
    // scale the mouse position, if necessary
    if (window->s_width != window->width) {
      window->mouse.x = (int)((double)window->mouse.real_x *
        ((double)window->s_width / (double)window->width) + 0.5);
    }
    
    if (window->s_height != window->height) {
      window->mouse.y = (int)((double)window->mouse.real_y *
        ((double)window->s_height / (double)window->height) + 0.5);
    }
    
    // Call update and render callbacks
    if (window->update) window->update();
    if (window->render) window->render();
    
    // Draw Frame //////////////////////////////////////////////////////////////
    SDL_GL_SwapWindow(window->sdl);
  }
  
  return 0;
}


/*
 * Close the window
 */
int S2D_Close(S2D_Window *window) {
  if (!window->close) {
    S2D_Log("Closing window", S2D_INFO);
    window->close = true;
  }
  return 0;
}


/*
 * Free all resources
 */
int S2D_FreeWindow(S2D_Window *window) {
  S2D_Close(window);
  SDL_GL_DeleteContext(window->glcontext);
  SDL_DestroyWindow(window->sdl);
  free(window);
  return 0;
}
