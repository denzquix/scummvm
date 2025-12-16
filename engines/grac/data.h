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

#ifndef GRAC_DATA_H
#define GRAC_DATA_H

#include "common/stream.h"

namespace Grac {

bool readTerminatedString(Common::SeekableReadStream *stream, Common::String &outString);

Common::SeekableReadStream* decompress(Common::SeekableReadStream *packedStream, bool isGrac2MainFile);

class Decompressor {
public:
  Decompressor(const uint32* packedStart, const uint32* packedEnd, byte* unpackedStart, byte* unpackedEnd);
private:
  const uint32* _packedEnd;
  const uint32* _packedStart;
  const uint32* _packedHead;
  byte* _unpackedEnd;
  byte* _unpackedStart;
  byte* _unpackedHead;
  uint32 _bitCount = 0;
  uint32 _bitBuf = 0;
  bool canReadBits(uint n);
  bool fillBitBuf();
  int readBit();
  int readBits(int n);
  bool writeByte(byte b);
  bool copy(uint distance, uint count);
public:
  bool readAll();
};

}

#endif
