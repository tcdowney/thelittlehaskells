enum spiSpeed {SPI_SLOW, SPI_MEDIUM, SPI_FAST};

void spiInit (SPI_TypeDef * SPIx);

int xchng_datablock(SPI_TypeDef *SPIx, int half, void *tbuf,
			   void *rbuf, unsigned count);

int dmaExgBytes(void *rbuf, void *tbuf, int half, unsigned count);

int spiReadWrite (SPI_TypeDef * SPIx, uint8_t *rbuf,
		   const uint8_t *tbuf, int cnt,
		   enum spiSpeed speed);

int spiReadWrite16 (SPI_TypeDef * SPIx, uint16_t *rbuf,
		     const uint16_t *tbuf, int cnt,
		     enum spiSpeed speed);
