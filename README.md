# STM32_GC9A01

GC9A01 display driver for STM32 and TouchGFX.

This driver requires STM32CubeMX generated `main.h` file and the HAL driver for SPI and GPIO.

This driver runs in RGB565 pixel format.

This driver uses hardware chip select pin.

Missing feature: backlight control, display rotation control.

## How to Use

### Preparation

Normally, `GC9A01.hpp` and `GC9A01.cpp` are all you need, but if your display is not working properly, this could be because this driver uses a striped-down version of the initialization sequence, in that case, try to use the full version at `initseq_full.cpp`. Go to `GC9A01.cpp` for more details.

#### Create a Display Object

```cpp
GC9A01::GC9A01(SPI_HandleTypeDef *hspi, GPIO_TypeDef *dcport, uint16_t dcpin)
```

* hspi: HAL SPI instance for transmissions
* dcport: GPIO port for Data/Command pin
* dcpin: GPIO pin for Data/Command pin

This is the bare minimum to run a display, if you want to use hardware reset:

```cpp
void GC9A01::setRSTpin(GPIO_TypeDef *rstport, uint16_t rstpin)
```

* rstport: GPIO port for reset pin
* rstpin: GPIO pin for reset pin

`setRSTpin` can be omitted if you don't want to use hardware reset.

#### Initialize the Display

```cpp
void GC9A01::begin(TXMethod method)
```

* method: trasmitting method for framebuffer.

This can be one of the follwing:

* `TXMethod::POLLING`
* `TXMethod::INTERRUPT`
* `TXMethod::DMA`

#### Example

```cpp
GC9A01 display(&hspi1, DISP_DC_GPIO_Port, DISP_DC_Pin);
display.setRSTpin(DISP_RST_GPIO_Port, DISP_RST_Pin);
display.begin(TXMethod::POLLING);
```

### Draw Image

After the initialization, you can now draw image on the display.

#### Select a Block

Specify a block of area on the screen you want to draw to.

```cpp
void GC9A01::selectBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
```

* x: starting point horizontal location (0-239 left to right)
* y: starting point vertical location (0-239 top to bottom)
* w: width of the block (1-240)
* h: height of the block (1-240)

Starting point is at the block's top-left corner.

#### Draw the Pixels

```cpp
void GC9A01::drawBitmap(uint8_t *pixels, uint16_t w, uint16_t h)
```

* pixels: a RGB565 pixel format array
* w: width of the image (1-240)
* h: height of the image (1-240)

The SPI transmission length of the HAL driver is limited to `uint16_t` (65535), a full 240x240 RGB565 image takes 240 * 240 * 2 (115200), so you may want to split your image, in that case you can run drawBitmap multiple times, it will pick up from where it left.

If you draw more pixels than the block you selected can handle, it will reset to the starting point and overwrite when it overflows.

If you want to manually reset to the starting point, run `selectBlock` again.

#### Example

```cpp
display.selectBlock(0, 0, 240, 240);
display.drawBitmap((uint8_t *)img, 240, 120);
display.drawBitmap((uint8_t *)&img[240 * 120 * 2], 240, 120);
```

## For TouchGFX Use

Go to example file `TouchGFXScreenControl.hpp` and `TouchGFXScreenControl.cpp` under TouchGFX folder for details.
