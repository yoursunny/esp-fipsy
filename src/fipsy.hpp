#ifndef FIPSY_HPP
#define FIPSY_HPP

#include <Arduino.h>
#include <SPI.h>
#include <array>
#include <bitset>

/** @brief Fipsy FPGA programmer. */
class Fipsy {
public:
  // Base FuseTable class with a fixed size
  template <size_t N>
  /**  @brief Fuse table of - get this from the .jed file e.g. QF73600 for MachXO2-256 and QF343936 for MachXO2-1200 */
  class FuseTable : public std::bitset<N> {
  public:
	enum class JedecError {
		OK,
		NO_STX,
		NO_ETX,
		BAD_QF,
		BAD_F,
		BAD_L,
		BAD_C,
		WRONG_CHECKSUM,
	  };
	/** @brief Parse fuse table from JEDEC file. */
	class JedecParser {
		public:
		  bool findStx() {
			return input.find('\x02');
		  }

		  bool skipField() {
			return input.find('*');
		  }

		  char readChar() {
			char ch;
			do {
			  if (input.readBytes(&ch, 1) != 1) {
				return 0;
			  }
			} while (ch == ' ' || ch == '\r' || ch == '\n');
			return ch;
		  }

		public:
		  Stream& input;
	};
	
    /**  @brief Compute fuse checksum. */
    uint16_t computeChecksum() const {
	  uint16_t c = 0x0000;
	  for (size_t i = 0; i < this->size(); ++i) {
		c += this->test(i) << (i % 8);
	  }
	  return c;
	};
	

	JedecError parseJedec(Stream& input) {
	  JedecParser parser{input};

	  bool ok = parser.findStx() && parser.skipField();
	  if (!ok) {
		return JedecError::NO_STX;
	  }

	  uint16_t fuseChecksum = 0xFFFF;

	  bool etx = false;
	  while (!etx) {
		char fieldId = parser.readChar();
		switch (fieldId) {
		  case 'Q':
			if (parser.readChar() == 'F') {
			  int qf = 0;
			  char ch;
			  while ((ch = parser.readChar()) != '*') {
				qf = qf * 10 + (ch - '0');
			  }
			  if (qf != this->size()) {
				return JedecError::BAD_QF;
			  }
			} else {
			  parser.skipField();
			}
			break;
		  case 'F': {
			switch (parser.readChar()) {
			  case '0':
				this->reset();
				break;
			  case '1':
				this->set();
				break;
			  default:
				return JedecError::BAD_F;
			}
			parser.skipField();
			break;
		  }
		  case 'L': {
			long addr = input.parseInt();
			for (bool stop = false; !stop;) {
			  switch (parser.readChar()) {
				case '0':
				  this->reset(addr++);
				  break;
				case '1':
				  this->set(addr++);
				  break;
				case '*':
				  stop = true;
				  break;
				default:
				  return JedecError::BAD_L;
			  }
			}
			break;
		  }
		  case 'C': {
			fuseChecksum = 0;
			char ch;
			while ((ch = parser.readChar()) != '*') {
			  switch (ch) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				  fuseChecksum = (fuseChecksum << 4) + (ch - '0');
				  break;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
				  fuseChecksum = (fuseChecksum << 4) + (ch - 'A' + 0x0A);
				  break;
				default:
				  return JedecError::BAD_C;
			  }
			}
		  }
		  case '\x03':
			etx = true;
			break;
		  default:
			if (!parser.skipField()) {
			  return JedecError::NO_ETX;
			}
			break;
		}
	  }

	  if (this->computeChecksum() != fuseChecksum) {
		return JedecError::WRONG_CHECKSUM;
	  }
	  return JedecError::OK;
	};


	/**
	* @brief Program fuse table.
	* @pre enable()
	*/
	bool program(Fipsy fipsy){
	  // erase flash
	  fipsy.spiTrans<4>({0x0E, 0x04, 0x00, 0x00});
	  fipsy.waitIdle();
	  if (fipsy.readStatus().fail()) {
		return false;
	  }

	  // set address to zero
	  fipsy.spiTrans<4>({0x46, 0x00, 0x00, 0x00});
	  fipsy.waitIdle();

	  // program pages
	  for (int i = 0; i < this->size(); i += 128) {
		std::array<uint8_t, 20> req;
		req.fill(0x00);
		req[0] = 0x70;
		req[3] = 0x01;
		for (int j = 0; j < 16; ++j) {
		  for (int k = 0; k < 8; ++k) {
			req[4 + j] |= this->test(i + j * 8 + k) << (7 - k);
		  }
		}
		fipsy.spiTrans<20>(req);
		fipsy.waitIdle();
	  }

	  // program DONE bit
	  fipsy.spiTrans<4>({0x5E, 0x00, 0x00, 0x00});
	  fipsy.waitIdle();
	  return !fipsy.readStatus().fail();
	}

  };

  /** @brief Status register value. */
  class Status {
  public:
    bool enabled() const {
      return v & (1 << 9);
    }

    bool busy() const {
      return v & (1 << 12);
    }

    bool fail() const {
      return v & (1 << 13);
    }

  public:
    uint32_t v;
  };

  /**
   * @brief Constructor.
   * @param spi the SPI bus.
   */
  explicit Fipsy(SPIClass& spi);

  /**
   * @brief Detect Fipsy.
   * @return Whether expected Device ID is found.
   */
  bool begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
  
  /**
   * @brief Detect Fipsy device ID.
   * @return Return any Device ID found.
   */
  uint32_t getID(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);

  /**
   * @brief Release SPI bus.
   * @pre begin()
   */
  void end();

  /**
   * @brief Read status register.
   * @pre begin()
   */
  Status readStatus();

  /**
   * @brief Enable offline configuration mode.
   * @pre begin()
   */
  bool enable();

  /**
   * @brief Disable configuration mode.
   * @pre enable()
   */
  void disable();

  /**
   * @brief Read Feature Row and FEABITS.
   * @pre enable()
   */
  void readFeatures(uint32_t& featureRow0, uint32_t& featureRow1, uint16_t& feabits);

  

private:
  template<int N>
  std::array<uint8_t, N> spiTrans(const std::array<uint8_t, N>& req);

  void waitIdle();

private:
  SPIClass& m_spi;
  int m_ss;
};

template<int N>
std::array<uint8_t, N>
Fipsy::spiTrans(const std::array<uint8_t, N>& req) {
  std::array<uint8_t, N> resp;
  digitalWrite(m_ss, LOW);
  m_spi.beginTransaction(SPISettings(400000, SPI_MSBFIRST, SPI_MODE0));
  m_spi.transferBytes(const_cast<uint8_t*>(req.data()), resp.data(), req.size());
  digitalWrite(m_ss, HIGH);
  m_spi.endTransaction();
  return resp;
}

#endif // FIPSY_HPP
