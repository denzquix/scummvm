/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"
#include "grac/amos/memorybank.h"

namespace Amos {

bool MemoryBank::load(Common::SeekableReadStream *fromStream) {
  if (!fromStream) {
    return false;
  }
  if (fromStream->readUint32BE() != MKTAG('A', 'm', 'B', 'k')) {
    return false;
  }
  uint16 bankNumber = fromStream->readUint16BE();
  uint16 memoryType = fromStream->readUint16BE();
  if (memoryType != MEMTYPE_CHIP_MEMORY && memoryType != MEMTYPE_FAST_MEMORY) {
    return false;
  }
  uint32 lengthAndFlags = fromStream->readUint32BE();
  uint32 bankLength = lengthAndFlags & LENGTH_MASK;
  uint32 flags = lengthAndFlags & FLAGS_MASK;
  if (bankLength < 8 || bankLength >= MAX_BANK_SIZE) {
    return false;
  }
  char bankName[8];
  if (fromStream->read(bankName, 8) != 8) {
    return false;
  }
  // bank length includes the name
  bankLength -= 8;
  byte *data = new byte[bankLength];
  if (fromStream->read(data, bankLength) != bankLength) {
    delete[] data;
    return false;
  }
  Type bankType = determineBankType(bankName);

  if (fromStream->err()) {
    delete[] data;
    return false;
  }

  _bankNumber = bankNumber;
  _bankType = bankType;
  _memoryType = (AmigaMemoryType)memoryType;
  _bankLength = bankLength;
  _flags = flags;
  memcpy(_bankName, bankName, 8);
  delete[] _data;
  _data = data;
  return true;
}

MemoryBank::Type MemoryBank::determineBankType(const char bankName[8]) {
  Common::String str(bankName, 8);
  switch (bankName[0]) {
    case 'M': {
      if (str == "Music   ") {
        return MEMBANK_MUSIC;
      }
      if (str == "Menu    ") {
        return MEMBANK_MENU;
      }
      return MEMBANK_UNKNOWN;
    }
    case 'A': {
      if (str == "Amal    ") {
        return MEMBANK_AMAL;
      }
      if (str == "Asm     ") {
        return MEMBANK_ASM;
      }
      return MEMBANK_UNKNOWN;
    }
    case 'P': {
      if (str == "Pac.Pic.") {
        return MEMBANK_PICTURE;
      }
      return MEMBANK_UNKNOWN;
    }
    case 'D': {
      if (str == "Datas   ") {
        return MEMBANK_DATA;
      }
      return MEMBANK_UNKNOWN;
    }
    case 'W': {
      if (str == "Work    ") {
        return MEMBANK_WORK;
      }
      return MEMBANK_UNKNOWN;
    }
    case 'S': {
      if (str == "Samples ") {
        return MEMBANK_SAMPLES;
      }
      return MEMBANK_UNKNOWN;
    }
    default: {
      return MEMBANK_UNKNOWN;
    }
  }
}

}