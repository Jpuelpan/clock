#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fontconfig/fontconfig.h>
#include <stdio.h>
#include <time.h>

const int GRID_COUNT = 15;
const int W_WIDTH = 600;
const int W_HEIGHT = 200;
const float FONT_SIZE = 1000.0;
const float SHADOW_OFFSET_X = 0.6;
const float SHADOW_OFFSET_Y = 0.8;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *whiteTextures[60];
SDL_Texture *blackTextures[60];

SDL_Color fgColor = {0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE};
SDL_Color bgColor = {0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE};

bool load_font(void *buff, char *font_name, size_t buff_size) {
  FcConfig *config = NULL;

  if (!FcInit()) {
    printf("Failed to initialize Fontconfig\n");
    return false;
  }

  config = FcInitLoadConfigAndFonts();
  if (config == NULL) {
    printf("Failed to load Fontconfig configuration\n");
    return false;
  }

  if (font_name == NULL) {
    font_name = "Mono:style=Bold";
  }

  FcResult result;
  FcPattern *pat = FcNameParse((const FcChar8 *)font_name);

  FcConfigSubstitute(config, pat, FcMatchKindBegin);
  FcDefaultSubstitute(pat);
  printf("Looking for font: %s\n", font_name);

  FcPattern *font = FcFontMatch(config, pat, &result);
  if (font) {
    FcChar8 *file = NULL;
    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
      memcpy(buff, file, buff_size);
    }
  }

  FcPatternDestroy(font);
  FcPatternDestroy(pat);
  FcConfigDestroy(config);
  FcFini();

  printf("Font found: %s\n", (char *)buff);
  return true;
}

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

void draw_circle(int x0, int y0, int r) {
  int x = r - 1;
  int y = 0;
  int dx = 1;
  int dy = 1;
  int err = dx - (r << 1);

  while (x >= y) {
    SDL_RenderLine(renderer, x + x0, y + y0, -x + x0, y + y0);
    SDL_RenderLine(renderer, y + x0, x + y0, -y + x0, x + y0);
    SDL_RenderLine(renderer, -x + x0, -y + y0, x + x0, -y + y0);
    SDL_RenderLine(renderer, -y + x0, -x + y0, y + x0, -x + y0);

    if (err <= 0) {
      y++;
      err += dy;
      dy += 2;
    }

    if (err > 0) {
      x--;
      dx += 2;
      err += dx - (r << 1);
    }
  }
}

void render_time_fragment(int *tm_part, SDL_FRect rect, int w, int h) {
  float shadow_x = ((w * SHADOW_OFFSET_X) / 100);
  float shadow_y = ((h * SHADOW_OFFSET_Y) / 100);

  SDL_FRect shadowRect;
  shadowRect.w = rect.w;
  shadowRect.h = rect.h;
  shadowRect.x = rect.x + shadow_x;
  shadowRect.y = rect.y + shadow_y;

  // Render shadow first
  SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
  SDL_RenderTexture(renderer, blackTextures[*tm_part], NULL, &shadowRect);

  // Render text later
  SDL_SetRenderDrawColor(renderer, fgColor.r, fgColor.g, fgColor.b, fgColor.a);
  SDL_RenderTexture(renderer, whiteTextures[*tm_part], NULL, &rect);
}

void render_time_separator(int x, float block_height, float w, float h) {
  float shadow_x = ((w * SHADOW_OFFSET_X) / 100.0f);
  float shadow_y = ((h * SHADOW_OFFSET_Y) / 100.0f);

  // Top circle
  SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
  draw_circle(x + shadow_x, block_height - (block_height / 9.0f) + shadow_y,
              ((w * 1.5f) / 100.0f));

  SDL_SetRenderDrawColor(renderer, fgColor.r, fgColor.g, fgColor.b, fgColor.a);
  draw_circle(x, block_height - (block_height / 9.0f), ((w * 1.5f) / 100.0f));

  // Bottom circle
  SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
  draw_circle(x + shadow_x, block_height + (block_height / 9.0f) + shadow_y,
              ((w * 1.5f) / 100.0f));

  SDL_SetRenderDrawColor(renderer, fgColor.r, fgColor.g, fgColor.b, fgColor.a);
  draw_circle(x, block_height + (block_height / 9.0f), ((w * 1.5f) / 100.0f));
}

int main(int argc, char *argv[]) {
  uint32_t win_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_TRANSPARENT |
                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;

  bool is_fullscreen = false;
  char font_name[512] = {};

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-fullscreen") == 0) {
      win_flags = win_flags | SDL_WINDOW_FULLSCREEN;
      is_fullscreen = true;
    } else if (strcmp(argv[i], "-font") == 0) {
      memcpy(font_name, argv[i + 1], sizeof(font_name));
    }
  }

  SDL_Init(SDL_INIT_VIDEO);

  if (!TTF_Init()) {
    SDL_Log("Failed to initialize SDL_ttf: %s\n", SDL_GetError());
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer("Clock", W_WIDTH, W_HEIGHT, win_flags,
                                   &window, &renderer)) {
    SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
    return 1;
  }

  char font_path[100];
  if (!load_font(&font_path, font_name, sizeof(font_path))) {
    SDL_Log("Failed to load font\n");
    return 1;
  }

  TTF_Font *font = TTF_OpenFont(font_path, FONT_SIZE);
  if (font == NULL) {
    SDL_Log("Failed to open font: %s\n", SDL_GetError());
    return 1;
  }

  for (int i = 0; i < 60; i++) {
    char content[10];
    sprintf(content, "%02d", i);

    SDL_Surface *text = TTF_RenderText_Blended(font, content, 0, fgColor);
    if (!text) {
      SDL_Log("Failed to texturize number: %s\n", SDL_GetError());
      return 1;
    }

    whiteTextures[i] = SDL_CreateTextureFromSurface(renderer, text);

    text = TTF_RenderText_Blended(font, content, 0, bgColor);
    if (!text) {
      SDL_Log("Failed to texturize number: %s\n", SDL_GetError());
      return 1;
    }
    blackTextures[i] = SDL_CreateTextureFromSurface(renderer, text);
    SDL_DestroySurface(text);
  }

  int canvas_width = 0, canvas_height = 0;
  bool shouldLoop = true;

  while (shouldLoop) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
          shouldLoop = false;
        } else if (event.key.key == SDLK_F) {
          is_fullscreen = !is_fullscreen;
          SDL_SetWindowFullscreen(window, is_fullscreen);
        }
        break;
      case SDL_EVENT_QUIT:
        shouldLoop = false;
        break;
      }
    }

    SDL_GetRenderOutputSize(renderer, &canvas_width, &canvas_height);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
    SDL_RenderClear(renderer);
    /* render_grid(&canvas_width, &canvas_height); */

    time_t now;
    time(&now);
    struct tm *now_tm = localtime(&now);

    float col_width = (float)canvas_width / (float)GRID_COUNT;
    float block_height = canvas_height / 2.0;

    SDL_FRect rect;
    rect.x = col_width * 2;
    rect.w = col_width * 3;
    rect.h = block_height;
    rect.y = canvas_height / 2.0 - block_height / 2.0;

    rect.x = col_width * 2;
    render_time_fragment(&now_tm->tm_hour, rect, canvas_width, canvas_height);
    render_time_separator((rect.x + rect.w + col_width / 2), block_height,
                          canvas_width, canvas_height);

    rect.x = rect.x + col_width * 4.0;
    render_time_fragment(&now_tm->tm_min, rect, canvas_width, canvas_height);
    render_time_separator((rect.x + rect.w + col_width / 2), block_height,
                          canvas_width, canvas_height);

    rect.x = rect.x + col_width * 4.0;
    render_time_fragment(&now_tm->tm_sec, rect, canvas_width, canvas_height);

    SDL_RenderPresent(renderer);
    SDL_Delay(100);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
