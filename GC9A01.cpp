#include <GC9A01.hpp>

#define GC9A01_SFTRST 0x01 // Software Reset (maybe, not documented)
#define GC9A01_EXTSLP 0x11 // Exit Sleep
#define GC9A01_DISPON 0x29 // Display ON
#define GC9A01_SETCA 0x2A  // Set Column Addresses
#define GC9A01_SETRA 0x2B  // Set Row Addresses
#define GC9A01_WRBUF 0x2C  // Write Framebuffer

// clang-format off
// Striped down version of initialization sequence,
// deleted some commands which seem to do nothing,
// if not working, try to use the full version at initseq_full.cpp
__attribute__((section(".rodata"))) static const uint8_t initseq[] = {
//  cmd, argc, argv...
	0xFE,  0,                                      // Inter register enable 1
	0xEF,  0,                                      // Inter register enable 2
	0x36,  1,  0x48,                               // Memory Access Control
	0x3A,  1,  0x05,                               // Color mode
	0xC3,  1,  0x13,                               // Power control 2
	0xC4,  1,  0x13,                               // Power control 3
	0xC9,  1,  0x22,                               // Power control 4
	0xF0,  6,  0x45, 0x09, 0x08, 0x08, 0x26, 0x2A, // Gamma 1
	0xF1,  6,  0x43, 0x70, 0x72, 0x36, 0x37, 0x6F, // Gamma 2
	0xF2,  6,  0x45, 0x09, 0x08, 0x08, 0x26, 0x2A, // Gamma 3
	0xF3,  6,  0x43, 0x70, 0x72, 0x36, 0x37, 0x6F, // Gamma 4
	0x66, 10,  0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, // Unknown but don't delete
	           0x10, 0x00, 0x00, 0x00,
	0x67, 10,  0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, // Unknown but don't delete
	           0x54, 0x10, 0x32, 0x98,
	0x35,  0,                                      // Tearing Effect Line ON
	0x21,  0                                       // Display Inversion OFF
};
// clang-format on

static inline HAL_StatusTypeDef SPI_TX_Polling_Wrapper(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size)
{
	return HAL_SPI_Transmit(hspi, pData, Size, 100);
}

void inline GC9A01::RESET(void)
{
	if (_rst)
	{
		HAL_GPIO_WritePin(_rstp, _rst, GPIO_PIN_RESET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(_rstp, _rst, GPIO_PIN_SET);
		HAL_Delay(150);
	}
	else
	{
		CMD(GC9A01_SFTRST);
		HAL_Delay(150);
	}
}

void inline GC9A01::CMD(uint8_t cmd)
{
	HAL_GPIO_WritePin(_dcp, _dc, GPIO_PIN_RESET);
	HAL_SPI_Transmit(_spi, &cmd, 1, 100);
}

void inline GC9A01::DATA(uint8_t *pdata, uint16_t size)
{
	HAL_GPIO_WritePin(_dcp, _dc, GPIO_PIN_SET);
	HAL_SPI_Transmit(_spi, pdata, size, 100);
}

GC9A01::GC9A01(SPI_HandleTypeDef *hspi, GPIO_TypeDef *dcport, uint16_t dcpin)
	: _spi(hspi), _dcp(dcport), _dc(dcpin)
{
}

void GC9A01::setRSTpin(GPIO_TypeDef *rstport, uint16_t rstpin)
{
	_rstp = rstport;
	_rst = rstpin;
}

void GC9A01::begin(TXMethod method)
{
	switch (method)
	{
	case TXMethod::POLLING:
		_transmitBuffer = SPI_TX_Polling_Wrapper;
		break;
	case TXMethod::INTERRUPT:
		_transmitBuffer = HAL_SPI_Transmit_IT;
		break;
	case TXMethod::DMA:
		_transmitBuffer = HAL_SPI_Transmit_DMA;
		break;
	default:
		_transmitBuffer = SPI_TX_Polling_Wrapper;
		break;
	}

	RESET();

	for (uint8_t i = 0; i < sizeof(initseq); i++)
	{
		CMD(initseq[i++]);
		if (initseq[i])
		{
			DATA((uint8_t *)&initseq[i + 1], initseq[i]);
			i += initseq[i];
		}
	}
	CMD(GC9A01_EXTSLP);
	HAL_Delay(120);
	CMD(GC9A01_DISPON);
	HAL_Delay(20);
}

void GC9A01::selectBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint8_t payload[8];

	payload[0] = 0;
	payload[1] = x & 0xFF;
	payload[2] = 0;
	payload[3] = (x + w - 1) & 0xFF;
	payload[4] = 0;
	payload[5] = y & 0xFF;
	payload[6] = 0;
	payload[7] = (y + h - 1) & 0xFF;

	CMD(GC9A01_SETCA);
	DATA(&payload[0], 4);
	CMD(GC9A01_SETRA);
	DATA(&payload[4], 4);

	CMD(GC9A01_WRBUF);
	HAL_GPIO_WritePin(_dcp, _dc, GPIO_PIN_SET);
}

void GC9A01::drawBitmap(uint8_t *pixels, uint16_t w, uint16_t h)
{
	_transmitBuffer(_spi, pixels, w * h * 2);
}
