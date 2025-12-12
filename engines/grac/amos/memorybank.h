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

#ifndef AMOS_MEMORYBANK_H
#define AMOS_MEMORYBANK_H

#include "common/stream.h"
#include "graphics/surface.h"
#include "graphics/palette.h"

namespace Amos {

enum AmigaMemoryType {
  MEMTYPE_CHIP_MEMORY = 0,
  MEMTYPE_FAST_MEMORY = 1,
};

class MemoryBank {

public:
  static const uint MAX_BANK_SIZE = 16*1024*1024;
  // Bits 28 and 29 are unused
  static const uint LENGTH_MASK = (1U << 28)-1;
  static const uint FLAGS_MASK = 0xC0000000;
  static const uint SCREEN_HEADER_MAGIC = 0x12031990;
  static const uint PICTURE_HEADER_MAGIC = 0x06071963;

  enum Type {
    MEMBANK_UNKNOWN = 0,
    MEMBANK_MUSIC = 1,
    MEMBANK_AMAL = 2,
    MEMBANK_MENU = 3,
    MEMBANK_DATA = 4,
    MEMBANK_WORK = 5,
    MEMBANK_ASM = 6,
    MEMBANK_PICTURE = 7,
    MEMBANK_SAMPLES = 8,
  };

  static const uint TRY_CHIP = (1U << 30);
  static const uint TRY_FAST = (1U << 31);

private:
  uint16 _bankNumber;
  AmigaMemoryType _memoryType;
  uint32 _bankLength;
  uint32 _flags;
  char _bankName[8];
  Type _bankType = MEMBANK_UNKNOWN;
  byte *_data = nullptr;
  static MemoryBank::Type determineBankType(const char bankName[8]);

public:
  ~MemoryBank() {
    delete[] _data;
  }

  MemoryBank() = default;
  MemoryBank(const MemoryBank&) = delete;
  MemoryBank& operator=(const MemoryBank&) = delete;

  bool load(Common::SeekableReadStream *fromStream);
  uint16 getBankNumber() const { return _bankNumber; }
  AmigaMemoryType getMemoryType() const { return _memoryType; }
  uint32 getBankLength() const { return _bankLength; }
  uint32 getFlags() const { return _flags; }
  const byte* getData() const { return _data; }
  MemoryBank::Type getBankType() const { return _bankType; }
  Common::String getName() const { return Common::String(_bankName, 8); }
  bool toPicture(Graphics::Surface& surf, Graphics::Palette& pal);

};

}

#endif
