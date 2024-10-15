#include "TouchGFXScreenControl.hpp"
#include <GC9A01.hpp>
#include <utility>

extern "C" void DisplayDriver_TransferCompleteCallback();
extern SPI_HandleTypeDef hspi1;
int transmitting = 0;
GC9A01 display(&hspi1, DISP_DC_GPIO_Port, DISP_DC_Pin);

void initializeDisplay()
{
	display.setRSTpin(DISP_RST_GPIO_Port, DISP_RST_Pin);
	display.begin(TXMethod::DMA);
}

extern "C" int touchgfxDisplayDriverTransmitActive(void)
{
	return transmitting;
}

extern "C" void touchgfxDisplayDriverTransmitBlock(uint8_t *pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	transmitting = 1;
	// Weirdly, the array that came from TouchGFX
	// has the order of the two bytes of every pixel swapped,
	// so we have to swap it back.
	for (uint16_t i = 0; i < w * h * 2; i += 2)
	{
		std::swap(pixels[i], pixels[i + 1]);
	}
	display.selectBlock(x, y, w, h);
	display.drawBitmap(pixels, w, h);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		transmitting = 0;
		DisplayDriver_TransferCompleteCallback();
	}
}
