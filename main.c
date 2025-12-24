#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int W_WIDTH = 600;
int W_HEIGHT = 200;
char FONT_PATH[] = "/usr/share/fonts/TTF/IosevkaTermNerdFont-ExtraBold.ttf";
float FONT_SIZE = 1000.0;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *numberTextures[60];

int main() {
  bool shouldLoop = true;
  SDL_Init(SDL_INIT_VIDEO);

  if (!TTF_Init()) {
    printf("Failed to initialize SDL_ttf: %s\n", SDL_GetError());
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
    printf("Failed to open font: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Color colorBlack = {0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE};
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0x00); // transparent window

  for (int i = 0; i < 60; i++) {
    char content[10];
    sprintf(content, "%02d", i);

    SDL_Surface *text = TTF_RenderText_Blended(font, content, 0, colorBlack);
    if (!text) {
      SDL_Log("Failed to texturize number: %s\n", SDL_GetError());
      return 1;
    }

    numberTextures[i] = SDL_CreateTextureFromSurface(renderer, text);
    SDL_DestroySurface(text);
  }

  const float scale = 0.8f;

  while (shouldLoop) {
    SDL_RenderClear(renderer);

    int w = 0, h = 0;
    SDL_GetRenderOutputSize(renderer, &w, &h);
    SDL_SetRenderScale(renderer, scale, scale);

    time_t now;
    time(&now);
    struct tm *now_tm = localtime(&now);

    SDL_FRect dst;

    dst.x = ((w / scale) - dst.w) / 2;
    dst.y = ((h / scale) - dst.h) / 2;
    SDL_GetTextureSize(numberTextures[now_tm->tm_sec], &dst.w, &dst.h);
    SDL_RenderTexture(renderer, numberTextures[now_tm->tm_sec], NULL, &dst);

    /* dst.x = 100.0 + dst.w; */
    /* dst.y = 100.0; */
    /* SDL_GetTextureSize(numberTextures[now_tm->tm_min], &dst.w, &dst.h); */
    /* SDL_RenderTexture(renderer, numberTextures[now_tm->tm_min], NULL, &dst);
     */

    /* dst.x = 100.0 + (dst.w * 2); */
    /* dst.y = 100.0; */
    /* SDL_GetTextureSize(numberTextures[now_tm->tm_sec], &dst.w, &dst.h); */
    /* SDL_RenderTexture(renderer, numberTextures[now_tm->tm_sec], NULL, &dst);
     */

    /* SDL_RenderClear(renderer);
    char content[100];
    sprintf(content, "%f", now);

    SDL_Surface *text = TTF_RenderText_Blended(font, content, 0, black);
    if (text) {
      SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text);
      SDL_DestroySurface(text);

      SDL_FRect dst;
      dst.x = 100.0;
      dst.y = 100.0;

      SDL_GetTextureSize(texture, &dst.w, &dst.h);
      SDL_RenderTexture(renderer, texture, NULL, &dst);
    } */

    /* SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); */
    /* SDL_RenderDebugTextFormat(renderer, 10.5, 11.0, "t: %f", now); */
    /* SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE); */
    /* SDL_RenderDebugTextFormat(renderer, 10.0, 10.0, "t: %f", now); */

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
