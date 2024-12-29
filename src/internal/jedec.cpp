#include "jedec.hpp"

namespace fipsy {

template<int base, typename I>
typename std::enable_if<std::is_integral<I>::value, bool>::type
appendDigit(I& n, char ch) {
  I digit = base;
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
      digit = static_cast<I>(ch - '0');
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      digit = static_cast<I>(ch - 'A' + 0x0A);
      break;
  }

  if (digit < 0 || digit >= base) {
    return false;
  }

  n = n * base + digit;
  return true;
}

class JedecParser {
public:
  JedecParser(Stream& input, JedecContent& output, FuseTable& fuseTable)
    : input(input)
    , output(output)
    , fuseTable(fuseTable) {}

  JedecError parse() {
    bool ok = findStx() && skipField();
    if (!ok) {
      return JedecError::NO_STX;
    }

#define HANDLE(field)                                                                              \
  if (!handle##field()) {                                                                          \
    return JedecError::BAD_##field;                                                                \
  }

    bool etx = false;
    while (!etx) {
      switch (readChar()) {
        case 'Q':
          if (readChar() != 'F') {
            skipField();
            break;
          }
          HANDLE(QF)
          break;
        case 'F': {
          HANDLE(F)
          break;
        }
        case 'L': {
          HANDLE(L)
          break;
        }
        case 'C': {
          HANDLE(C)
          break;
        }
        case 'E': {
          HANDLE(E)
          break;
        }
        case '\x03':
          etx = true;
          break;
        default:
          if (!skipField()) {
            return JedecError::NO_ETX;
          }
          break;
      }
    }
#undef HANDLE

    if (fuseTable.computeChecksum() != fuseChecksum) {
      return JedecError::WRONG_CHECKSUM;
    }
    return JedecError::OK;
  }

private:
  bool findStx() {
    return input.find('\x02');
  }

  bool skipField() {
    return input.find('*');
  }

  char readChar(bool allowWhitespace = false) {
    char ch;
    do {
      if (input.readBytes(&ch, 1) != 1) {
        return 0;
      }
    } while (!allowWhitespace && isspace(ch));
    return ch;
  }

  bool handleQF() {
    qf = 0;
    char ch;
    while ((ch = readChar()) != '*') {
      if (!appendDigit<10>(qf, ch)) {
        return false;
      }
    }

    return qf == static_cast<int>(fuseTable.size());
  }

  bool handleF() {
    char ch = readChar();
    switch (ch) {
      case '0':
      case '1':
        break;
      default:
        return false;
    }
    fuseTable.assign(qf, ch - '0');
    skipField();
    return true;
  }

  bool handleL() {
    int addr = 0;
    while (true) {
      char ch = readChar(true);
      if (isspace(ch)) {
        break;
      }
      if (!appendDigit<10>(addr, ch)) {
        return false;
      }
    }

    while (true) {
      char ch = readChar();
      switch (ch) {
        case '0':
        case '1':
          break;
        case '*':
          return true;
        default:
          return false;
      }
      if (addr >= qf) {
        return false;
      }
      fuseTable[addr++] = ch - '0';
    }
  }

  bool handleC() {
    fuseChecksum = 0;
    char ch;
    while ((ch = readChar()) != '*') {
      if (!appendDigit<16>(fuseChecksum, ch)) {
        return false;
      }
    }
    return true;
  }

  bool handleE() {
    for (auto& b : output.features) {
      b = 0;
      for (int i = 0; i < 8; ++i) {
        if (!appendDigit<2>(b, readChar())) {
          return false;
        }
      }
    }
    return true;
  }

public:
  Stream& input;
  JedecContent& output;
  FuseTable& fuseTable;

private:
  int qf = 0;
  uint16_t fuseChecksum = 0xFFFF;
};

JedecContent
parseJedec(Stream& input, FuseTable& fuseTable) {
  JedecContent output;
  JedecParser parser(input, output, fuseTable);
  output.error = parser.parse();
  return output;
}

} // namespace fipsy
