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

#include "common/stream.h"
#include "common/memstream.h"
#include "common/system.h"
#include "grac/data.h"

namespace Grac {

bool readTerminatedString(Common::SeekableReadStream *stream, Common::String &outString) {
  outString.clear();
  for (;;) {
    byte c = stream->readByte();
    if (stream->err() || stream->eos()) {
      outString.clear();
      return false;
    }
    if (c == 0xD7) {
      return true;
    }
    outString += (char)c;
  }
}

Decompressor::Decompressor(const uint32* packedStart, const uint32* packedEnd, byte* unpackedStart, byte* unpackedEnd) :
  _packedEnd(packedEnd),
  _packedStart(packedStart),
  _packedHead(packedEnd),
  _unpackedEnd(unpackedEnd),
  _unpackedStart(unpackedStart),
  _unpackedHead(unpackedEnd) {
}
bool Decompressor::canReadBits(uint n) {
  return (_bitCount + 8 * 4 * (_packedHead - _packedStart)) >= n;
}
bool Decompressor::fillBitBuf() {
  if (_packedHead <= _packedStart) {
    return false;
  }
  _bitBuf = *--_packedHead;
  _bitCount = 32;
  return true;
}
int Decompressor::readBit() {
  if (_bitCount == 0 && !fillBitBuf()) {
    return -1;
  }
  uint b = _bitBuf & 1;
  _bitBuf >>= 1;
  _bitCount--;
  return b;
}
int Decompressor::readBits(int n) {
  int v = 0;
  for (int i = 0; i < n; i++) {
    v = (v << 1) | readBit();
  }
  return v;
}
bool Decompressor::writeByte(byte b) {
  if (_unpackedHead <= _unpackedStart) {
    return false;
  }
  *--_unpackedHead = b;
  return true;
}
bool Decompressor::copy(uint distance, uint count) {
  if ((_unpackedHead - count) < _unpackedStart) {
    return false;
  }
  byte* copyHead = _unpackedHead + distance;
  if (copyHead > _unpackedEnd) {
    return false;
  }
  while (count != 0) {
    *--_unpackedHead = *--copyHead;
    count--;
  }
  return true;
}
bool Decompressor::readAll() {
  // initiate bit buffer using anchor bit for alignment
  if (!fillBitBuf()) {
    return false;
  }
  if (_bitBuf == 0) {
    // no anchor bit!
    return false;
  }
  for (_bitCount = 31; _bitCount >= 0; _bitCount--) {
    if (_bitBuf & (1U << _bitCount)) {
      // anchor bit found
      break;
    }
  }
  // mask off anchor bit (not strictly necessary)
  _bitBuf &= (1U << _bitCount)-1;
  while (_unpackedHead > _unpackedStart) {
    if (!canReadBits(2)) {
      return false;
    }
    uint distance, count;
    switch (readBit()) {
      case 0: {
        switch (readBit()) {
          case 0: {
            if (!canReadBits(3)) {
              return false;
            }
            count = readBits(3) + 1;
            if (!canReadBits(8 * count)) {
              return false;
            }
            while (count > 0) {
              if (!writeByte(readBits(8))) {
                return false;
              }
              count--;
            }
            break;
          }
          case 1: {
            if (!canReadBits(8)) {
              return false;
            }
            distance = readBits(8);
            if (!copy(distance, 2)) {
              return false;
            }
            break;
          }
        }
        break;
      }
      case 1: {
        if (!canReadBits(2)) {
          return false;
        }
        switch (readBit()) {
          case 0: {
            switch (readBit()) {
              case 0: {
                if (!canReadBits(9)) {
                  return false;
                }
                distance = readBits(9);
                if (!copy(distance, 3)) {
                  return false;
                }
                break;
              }
              case 1: {
                if (!canReadBits(10)) {
                  return false;
                }
                distance = readBits(10);
                if (!copy(distance, 4)) {
                  return false;
                }
                break;
              }
            }
            break;
          }
          case 1: {
            switch (readBit()) {
              case 0: {
                if (!canReadBits(20)) {
                  return false;
                }
                count = readBits(8) + 1;
                distance = readBits(12);
                if (!copy(distance, count)) {
                  return false;
                }
                break;
              }
              case 1: {
                if (!canReadBits(8)) {
                  return false;
                }
                count = readBits(8) + 9;
                if (!canReadBits(count * 8)) {
                  return false;
                }
                while (count > 0) {
                  if (!writeByte(readBits(8))) {
                    return false;
                  }
                  count--;
                }
                break;
              }
            }
            break;
          }
        }
        break;
      }
    }
  }
  if (canReadBits(1)) {
    return false;
  }
  return true;
}

Common::SeekableReadStream* decompress(Common::SeekableReadStream *packedStream, bool isGrac2MainFile) {
  const uint startPos = isGrac2MainFile ? 8 : 4;
  if (!packedStream) {
    warning("Unable to unpack: Input stream is null");
    return nullptr;
  }
  if (isGrac2MainFile && packedStream->readUint32BE() != MKTAG('G', 'R', '2', '0')) {
    warning("Unable to unpack: GR20 tag not found");
    delete packedStream;
    return nullptr;
  }
  int64 streamLen = packedStream->size();
  if (streamLen < (startPos + 8) || streamLen % 4 != 0) {
    warning("Unable to unpack: Invalid length (%d)", streamLen);
    delete packedStream;
    return nullptr;
  }
  if (!packedStream->seek(-8, SEEK_END)) {
    warning("Unable to unpack: Failed to seek");
    delete packedStream;
    return nullptr;
  }
  uint32 checksum = packedStream->readUint32BE();
  uint32 unpackedLength = packedStream->readUint32BE();
  if (unpackedLength > 16*1024*1024) {
    warning("Unable to unpack: Unpacked length too big (%d)", unpackedLength);
    delete packedStream;
    return nullptr;
  }
  if (!packedStream->seek(startPos, SEEK_SET)) {
    warning("Unable to unpack: Failed to seek");
    delete packedStream;
    return nullptr;
  }
  uint32 sum = 0;
  for (int64 pos = startPos; pos < streamLen-8; pos += 4) {
    sum ^= packedStream->readUint32BE();
  }
  if (sum != checksum) {
    warning("Unable to unpack: Failed checksum (expected %x, got %x)", checksum, sum);
    delete packedStream;
    return nullptr;
  }
  uint32 packedLength = ((uint32)streamLen - (startPos + 8)) / 4;
  uint32* packed = new uint32[packedLength];
  if (!packedStream->seek(startPos) || packedStream->read(packed, packedLength*4) != packedLength*4 || packedStream->err()) {
    warning("Unable to unpack: Failed to read payload");
    delete[] packed;
    delete packedStream;
    return nullptr;
  }
  delete packedStream;
  for (uint32 packed_i = 0; packed_i < packedLength; packed_i++) {
    packed[packed_i] = FROM_BE_32(packed[packed_i]);
  }
  byte* unpacked = new byte[unpackedLength];
  Decompressor decomp(packed, packed + packedLength, unpacked, unpacked + unpackedLength);
  if (!decomp.readAll()) {
    warning("Unable to unpack: Decompression stream error");
    delete[] packed;
    delete[] unpacked;
    return nullptr;
  }
  delete[] packed;
  return new Common::MemoryReadStream(unpacked, unpackedLength, DisposeAfterUse::YES);
}

}
