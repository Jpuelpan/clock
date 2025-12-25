#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

const int GRID_COUNT = 15;
const int W_WIDTH = 600;
const int W_HEIGHT = 200;
const char FONT_PATH[] = "/usr/share/fonts/TTF/IosevkaTermNerdFont-Regular.ttf";
const float FONT_SIZE = 1000.0;
const int SHADOW_OFFSET_X = -20;
const int SHADOW_OFFSET_Y = -25;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *whiteTextures[60];
SDL_Texture *blackTextures[60];

void render_grid(int *width, int *height) {
  float col_width = (float)*width / (float)GRID_COUNT;

  for (int i = 0; i < GRID_COUNT; i++) {
    SDL_FRect col_rect;
    col_rect.w = col_width;
    col_rect.h = *height;

    col_rect.x = col_width * i;
    col_rect.y = 0;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00,
                           i % 2 == 0 ? 0x00 : 0xAA);
    SDL_RenderFillRect(renderer, &col_rect);
  }
}

int main() {
  bool shouldLoop = true;
  SDL_Init(SDL_INIT_VIDEO);

  if (!TTF_Init()) {
    SDL_Log("Failed to initialize SDL_ttf: %s\n", SDL_GetError());
    return 1;
  }

  uint32_t win_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_TRANSPARENT |
                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;

  if (!SDL_CreateWindowAndRenderer("Clock", W_WIDTH, W_HEIGHT, win_flags,
                                   &window, &renderer)) {
    SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
    return 1;
  }

  TTF_Font *font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
  if (font == NULL) {
    SDL_Log("Failed to open font: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Color white = {0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE};
  SDL_Color black = {0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE};

  for (int i = 0; i < 60; i++) {
    char content[10];
    sprintf(content, "%02d", i);

    SDL_Surface *text = TTF_RenderText_Blended(font, content, 0, white);
    if (!text) {
      SDL_Log("Failed to texturize number: %s\n", SDL_GetError());
      return 1;
    }

    whiteTextures[i] = SDL_CreateTextureFromSurface(renderer, text);

    text = TTF_RenderText_Blended(font, content, 0, black);
    if (!text) {
      SDL_Log("Failed to texturize number: %s\n", SDL_GetError());
      return 1;
    }
    blackTextures[i] = SDL_CreateTextureFromSurface(renderer, text);
    SDL_DestroySurface(text);
  }

  SDL_Surface *text = TTF_RenderText_Blended(font, ":", 0, white);
  if (!text) {
    SDL_Log("Failed to texturize colon: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Texture *colon = SDL_CreateTextureFromSurface(renderer, text);
  SDL_DestroySurface(text);

  int canvas_width = 0, canvas_height = 0;

  while (shouldLoop) {
    SDL_GetRenderOutputSize(renderer, &canvas_width, &canvas_height);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0x00); // transparent window
    SDL_RenderClear(renderer);

    /* render_grid(&canvas_width, &canvas_height); */

    time_t now;
    time(&now);
    struct tm *now_tm = localtime(&now);

    float col_width = (float)canvas_width / (float)GRID_COUNT;
    float block_height = canvas_height / 2.0;

    SDL_FRect rect, shadowRect;
    rect.w = col_width * 3;
    rect.h = block_height;
    rect.y = canvas_height / 2.0 - block_height / 2.0;

    shadowRect.w = rect.w;
    shadowRect.h = rect.h;

    rect.x = col_width * 2;
    shadowRect.x = rect.x + SHADOW_OFFSET_X;
    shadowRect.y = rect.y + SHADOW_OFFSET_Y;

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderTexture(renderer, blackTextures[now_tm->tm_hour], NULL, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderTexture(renderer, whiteTextures[now_tm->tm_hour], NULL,
                      &shadowRect);

    rect.x = rect.x + col_width * 4.0;
    shadowRect.x = rect.x + SHADOW_OFFSET_X;
    shadowRect.y = rect.y + SHADOW_OFFSET_Y;
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderTexture(renderer, blackTextures[now_tm->tm_min], NULL, &rect);
    SDL_RenderTexture(renderer, whiteTextures[now_tm->tm_min], NULL,
                      &shadowRect);

    rect.x = rect.x + col_width * 4.0;
    shadowRect.x = rect.x + SHADOW_OFFSET_X;
    shadowRect.y = rect.y + SHADOW_OFFSET_Y;
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderTexture(renderer, blackTextures[now_tm->tm_sec], NULL, &rect);
    SDL_RenderTexture(renderer, whiteTextures[now_tm->tm_sec], NULL,
                      &shadowRect);

    SDL_RenderPresent(renderer);
    SDL_Delay(100);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_ESCAPE) {
          shouldLoop = false;
        }
        break;
      case SDL_EVENT_QUIT:
        shouldLoop = false;
        break;
      }
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
