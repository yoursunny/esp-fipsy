#include "jedec.hpp"
#include <stdio.h>

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

  char readChar() {
    char ch;
    do {
      if (input.readBytes(&ch, 1) != 1) {
        return 0;
      }
    } while (ch == ' ' || ch == '\r' || ch == '\n');
    return ch;
  }

  bool handleQF() {
    qf = 0;
    qfSize = 0;
    char ch;
    while ((ch = readChar()) != '*') {
      if (!appendDigit<10>(qf, ch)) {
        return false;
      }
      ++qfSize;
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
    fuseTable.resize(qf, ch - '0');
    skipField();
    return true;
  }

  bool handleL() {
    printf("Doing function: HandleL\n");
    int addr = 0;
    while (true) {
      char ch;
      // readChar() does not let us capture terminal strings, so doing it manually here
      // this fixes a bug where readChar() denpends on sizeQf length - which is NOT related
      // to the length of L characters. That is, length of QF chars != length of L chars.
      input.readBytes(&ch, 1);

      // printf("Char here is [%c] booleab pass: %B\n",ch, ch=='0');

      if (!(ch >= '0' && ch <= '9')) {
        break;
      }

      if (!appendDigit<10>(addr, ch)) {
        return false;
      }
    }

    printf("Starting addr: %d\n", addr);

    while (true) {
      char ch = readChar();
      switch (ch) {
        case '0':
        case '1':
          break;
        case '*':
          return true;
        default:
          printf("Failure F2: Unrecognized character\n");
          return false;
      }

      // TODO: Fix this check so it passes for Fipsy V2 - MachX02-1200
      /*
      if (addr >= qf) {
        printf("Failure F3: Addr: %d and qf: %d for character index: %d\n", addr, qf, index);
        return false;
      }
      */
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
  int qfSize = 0;
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
