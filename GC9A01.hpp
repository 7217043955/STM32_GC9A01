#pragma once
#include <main.h>

enum class TXMethod : uint8_t
{
	POLLING,
	INTERRUPT,
	DMA
};

class GC9A01
{
private:
	SPI_HandleTypeDef *_spi;
	GPIO_TypeDef *_dcp;
	uint16_t _dc;
	GPIO_TypeDef *_rstp = nullptr;
	uint16_t _rst = 0;
	HAL_StatusTypeDef (*_transmitBuffer)(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size) = nullptr;

	void RESET(void);
	void CMD(uint8_t cmd);
	void DATA(uint8_t *pdata, uint16_t size);

public:
	GC9A01(SPI_HandleTypeDef *hspi, GPIO_TypeDef *dcport, uint16_t dcpin);
	void setRSTpin(GPIO_TypeDef *rstport, uint16_t rstpin);
	void begin(TXMethod method);

	void selectBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	void drawBitmap(uint8_t *pixels, uint16_t w, uint16_t h);
};
