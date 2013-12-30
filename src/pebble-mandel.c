#include <pebble.h>

#include "dither.h"
#define USE_FIXED_POINT 1
//#undef __arm__
#ifdef __arm__
  #pragma message "__arm__ enabled"
#endif
#include "math-sll.h"

#define MAX_ITERATIONS 123

static Window *mandel_window;
static GBitmap *cover = NULL;
static Layer *render_layer;
static int xres = 0;
static int yres = 0;

static sll minX;
static sll maxX;
static sll minY;
static sll maxY;

static uint8_t* gray4_framebuffer = NULL;//[144*168 / 2]; //grayscale 4-bit

#define DRAW_PIXEL( framebuffer, x, y, color ) \
    if (x % 2 == 1) {\
      framebuffer[((y*xres + x) / 2)] = (framebuffer[((y*xres + x) / 2)] & 0xF0) | (color & 0x0F); \
    } else {\
      framebuffer[((y*xres + x) / 2)] = (framebuffer[((y*xres + x) / 2)] & 0x0F) | ((color & 0x0F) << 4);\
    }



static uint8_t mandelbrot_calcpixel(sll x, sll y) {
    int iterations = 0;
    sll zx = int2sll(0);
    sll zy = int2sll(0);
    sll temp_a = int2sll(0);
    sll maxdist = int2sll(4);

    for (iterations = 0; iterations < MAX_ITERATIONS; iterations++){
        temp_a = sllmul2(zx); 
        zx = slladd(sllsub(sllmul(zx,zx), sllmul(zy,zy)),x);
        zy = slladd(sllmul(temp_a, zy),y);
        if( slladd(sllmul(zx,zx), sllmul(zy,zy)) >= maxdist ) break;
    }
    //Dont exceed iterations range because of for loop
    //return (iterations == MAX_ITERATIONS) ? (MAX_ITERATIONS - 1)>>1 : iterations >>1;
    return iterations >> 3;
}

static void draw_mandel(Layer* layer, GContext *ctx) {
  GBitmap* bitmap = (GBitmap*)ctx;
  uint32_t* framebuffer = (uint32_t*)bitmap->addr;
  GRect layer_bounds = layer_get_bounds(layer);

  //quick draw cover image once
  if(cover){
    graphics_draw_bitmap_in_rect(ctx, cover, (GRect) { .origin = { 0, 0 }, .size = layer_bounds.size });
    gbitmap_destroy(cover);
    cover = NULL;
    psleep(3 * 1000);
    return;
  }

  xres = layer_bounds.size.w;
  yres = layer_bounds.size.h;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Bounding: %d %d %d %d", 
    layer_bounds.origin.x, layer_bounds.origin.y,
    layer_bounds.size.w, layer_bounds.size.h);

  sll dx, dy;
  int x, y, color = 0;
  static sll zoom = int2sll(1.2);

  time_t start_time = time(NULL);
  uint16_t start_time_ms = time_ms(NULL, NULL);
  time_t end_time;
  uint16_t end_time_ms;

  dx = slldiv(sllsub(maxX,minX), int2sll(xres));
  dy = slldiv(sllsub(maxY,minY), int2sll(yres));
  dx = sllmul(dx,zoom);
  dy = sllmul(dy,zoom);
  for (y=0; y<yres; y++){
    for(x=0; x<xres; x++) {
      color = mandelbrot_calcpixel(
        slladd(minX, sllmul(int2sll(x-xres/2),dx)), 
        slladd(minY, sllmul(int2sll(y-yres/2),dy)));
      // Small filtering for low gray values
      DRAW_PIXEL( gray4_framebuffer, x, y, color>3?color:0);
    }
    // small sleep or crashes (lockup on watch)
    //if (y % 4 == 0) psleep(1);
  }
  floyd_steinberg_dither(gray4_framebuffer, (uint8_t*)framebuffer, -1);
  end_time = time(NULL);
  end_time_ms = time_ms(NULL, NULL);
  zoom = sllmul(zoom,dbl2sll(0.90));
  int time_dur = 
    (end_time * 1000 + end_time_ms) -
    (start_time * 1000 + start_time_ms); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Frame took %d:%d", 
    (int)(time_dur / 1000 ), (int)(time_dur % 1000)); 

}


static void register_timer(void* data) {
  app_timer_register(1000, register_timer, data);
  layer_mark_dirty(render_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  render_layer = layer_create(bounds);
  layer_set_update_proc(render_layer, draw_mandel);
  layer_add_child(window_layer, render_layer);

  register_timer(NULL);
}

static void window_unload(Window *window) {
}

static void init(void) {
  //Load startup cover
  cover = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COVER);

  //Allocate 4-bit grayscale buffer
  gray4_framebuffer = malloc(144*168 / 2);
  minX = dbl2sll(0.366);//-2.5);
  maxX = dbl2sll(0.376);//1.0);
  minY = dbl2sll(0.088);//-1.0);
  maxY = dbl2sll(0.100);//1.0);

  mandel_window = window_create();
  window_set_fullscreen(mandel_window, true);
  window_set_window_handlers(mandel_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(mandel_window, animated);
}

static void deinit(void) {
  window_destroy(mandel_window);
  free(gray4_framebuffer);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", mandel_window);

  app_event_loop();
  deinit();
}
