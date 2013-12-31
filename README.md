pebble-mandelbrot-generator
===========================
Pebble smartwatch app generating mandelbrot set, floyd-steinberg dithered, sll fastmath.

![cover](http://mhungerford.github.io/pebble-mandelbrot-generator/pebble_cover.png)

<a href="https://github.com/mhungerford/pebble-mandelbrot-generator/raw/master/pebble-mandelbrot-generator.pbw">download</a>

### Framebuffer access
Direct framebuffer access is used for fast pixel manipulation.  Refer to the code for framebuffer access through the update layer function.

### 4-bit Dithering
This app uses 4-bit to 1-bit Floyd-Steinberg dithering to allow the mandelbrot generator to create colors, and display the final image on the Pebble's black-and-white EPaper screen.  


### SLL Fastmath
Original version used gcc's softfloat (provided by newlib), which generated a mandelbrot frame every 40 seconds.  Using SLL fastmath (from picoGL/tinyGL), a mandelbrot frame is generated in less than 4 seconds, for a 10x speedup, which makes watching the mandelbrot much less boring ;)

![mandel1](http://mhungerford.github.io/pebble-mandelbrot-generator/mandel1.png) | 
![mandel2](http://mhungerford.github.io/pebble-mandelbrot-generator/mandel2.png) | 
![mandel3](http://mhungerford.github.io/pebble-mandelbrot-generator/mandel3.png) | 
![mandel4](http://mhungerford.github.io/pebble-mandelbrot-generator/mandel4.png)

Mandelbrot Pebble Watchapp for FW 2.0 : <a href="https://github.com/mhungerford/pebble-mandelbrot-generator/raw/master/pebble-mandelbrot-generator.pbw">download</a>
