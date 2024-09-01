#include "fusetable.hpp"

namespace fipsy {

uint16_t
FuseTable::computeChecksum() const {
  uint16_t c = 0x0000;
  for (size_t i = 0; i < size(); ++i) {
    c += static_cast<uint16_t>((*this)[i]) << (i % 8);
  }
  return c;
}

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

JedecError
parseJedec(Stream& input, FuseTable& fuseTable) {
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
          if (qf != fuseTable.size()) {
            return JedecError::BAD_QF;
          }
        } else {
          parser.skipField();
        }
        break;
      case 'F': {
        bool value = false;
        switch (parser.readChar()) {
          case '0':
            value = false;
            break;
          case '1':
            value = true;
            break;
          default:
            return JedecError::BAD_F;
        }
        size_t size = fuseTable.size();
        fuseTable.clear();
        fuseTable.resize(size, value);
        parser.skipField();
        break;
      }
      case 'L': {
        long addr = input.parseInt();
        for (bool stop = false; !stop;) {
          switch (parser.readChar()) {
            case '0':
              fuseTable[addr++] = false;
              break;
            case '1':
              fuseTable[addr++] = true;
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
        break;
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

  if (fuseTable.computeChecksum() != fuseChecksum) {
    return JedecError::WRONG_CHECKSUM;
  }
  return JedecError::OK;
}

} // namespace fipsy
