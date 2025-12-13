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
#include "common/stream.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
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

void amigaColor(uint16 amigaColor, byte& red, byte& green, byte& blue) {
  red =   ((amigaColor >> 8) & 0x0f) * 0x11;
  green = ((amigaColor >> 4) & 0x0f) * 0x11;
  blue =  ((amigaColor     ) & 0x0f) * 0x11;
}
 
bool MemoryBank::toPicture(Graphics::Surface& surf, Graphics::Palette& pal) {
  if (_bankType != MEMBANK_PICTURE) {
    return false;
  }
  Common::ScopedPtr<Common::MemoryReadStream> ptr(new Common::MemoryReadStream(_data, _bankLength));
  // read screen header
  if (ptr->readUint32BE() != SCREEN_HEADER_MAGIC) {
    warning("screen header signature not found");
    return false;
  }
  uint16 screenWidth = ptr->readUint16BE();
  uint16 screenHeight = ptr->readUint16BE();
  uint16 hardwareX = ptr->readUint16BE();
  uint16 hardwareY = ptr->readUint16BE();
  uint16 hardwareScreenWidth = ptr->readUint16BE();
  uint16 hardwareScreenHeight = ptr->readUint16BE();
  uint16 hardwareXOffset = ptr->readUint16BE();
  uint16 hardwareYOffset = ptr->readUint16BE();
  uint16 bplcon0 = ptr->readUint16BE();
  uint16 colors = ptr->readUint16BE();
  if (colors > 32) {
    warning("too many colors: %d", colors);
    return false;
  }
  uint16 screenBitplanes = ptr->readUint16BE();
  pal.resize(colors, false);
  for (uint i = 0; i < colors; i++) {
    byte r,g,b;
    amigaColor(ptr->readUint16BE(), r, g, b);
    pal.set(i, r, g, b);
  }
  if (!ptr->skip((32 - colors) * 2)) {
    warning("failed to skip");
    pal.clear();
    return false;
  }
  // read picture header
  int64 picHeaderOffset = ptr->pos();
  if (picHeaderOffset < 0 || picHeaderOffset >= _bankLength) {
    warning("invalid picHeaderOffset: %d", picHeaderOffset);
    pal.clear();
    return false;
  }
  if (ptr->readUint32BE() != PICTURE_HEADER_MAGIC) {
    warning("picture header signature not found");
    pal.clear();
    return false;
  }
  uint16 xOffsetBytes = ptr->readUint16BE();
  uint16 yOffsetLines = ptr->readUint16BE();
  uint16 picWidthBytes = ptr->readUint16BE();
  uint16 picHeightLumps = ptr->readUint16BE();
  uint16 linesPerLump = ptr->readUint16BE();
  uint16 picBitplanes = ptr->readUint16BE();
  if (picBitplanes < 1 || picBitplanes > 6) {
    warning("invalid number of bitplanes: %d", picBitplanes);
    pal.clear();
    return false;
  }
  int64 rle1Offset = picHeaderOffset + ptr->readUint32BE();
  int64 rle2Offset = picHeaderOffset + ptr->readUint32BE();
  // We have already ensured that picHeaderOffset's value fits in a
  // uint32, so rle1Offset and rle2Offset (as the sum of two uint32s
  // in an int64) cannot have overflowed, so cannot be negative
  if (rle1Offset >= _bankLength || rle2Offset >= _bankLength) {
    warning("invalid offset");
    pal.clear();
    return false;
  }
  const byte* const rle1Start = _data + rle1Offset;
  const byte* const rle2Start = _data + rle2Offset;
  int64 packedOffset = ptr->pos();
  if (packedOffset < 0 || packedOffset >= _bankLength) {
    warning("invalid offset");
    pal.clear();
    return false;
  }
  const byte* const packedStart = _data + packedOffset;
  const byte* const dataEnd = _data + _bankLength;
  if (packedStart >= dataEnd || rle1Start >= dataEnd || rle2Start >= dataEnd) {
    warning("data underrun");
    pal.clear();
    return false;
  }
  uint32 height = picHeightLumps * linesPerLump;
  uint32 width = picWidthBytes * 8;
  if (width < 1 || height < 1 || width >= 2048 || height >= 2048) {
    warning("invalid dimensions: %dx%d", width, height);
    pal.clear();
    return false;
  }
  surf.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
  byte *pixels = (byte*)surf.getPixels();
  const byte* packed = packedStart;
  const byte* rle1 = rle1Start;
  const byte* rle2 = rle2Start;
  byte rle1BitBuf = *rle1++, rle1BitCount = 8;
  byte rle2BitBuf = *rle2++, rle2BitCount = 8;
  byte picByte = *packed++;
  if (rle2BitBuf & 0x80) {
    if (rle1 >= dataEnd) {
      warning("ran out of data reading rle1");
      pal.clear();
      surf.free();
      return false;
    }
    rle1BitBuf = *rle1++;
  }
  rle2BitBuf <<= 1;
  rle2BitCount--;
  byte prevRle1BitBuf = rle1BitBuf;
  for (uint bitplane_i = 0; bitplane_i < picBitplanes; bitplane_i++) {
    byte planed_bit = 1 << bitplane_i;
    for (uint lump_i = 0; lump_i < picHeightLumps; lump_i++) {
      for (uint byte_i = 0; byte_i < picWidthBytes; byte_i++) {
        for (uint line_i = 0; line_i < linesPerLump; line_i++) {
          if (rle1BitBuf & 0x80) {
            if (packed >= dataEnd) {
              warning("packed data overrun");
              pal.clear();
              surf.free();
              return false;
            }
            picByte = *packed++;
          }
          for (byte pixel_bit = 0; pixel_bit < 8; pixel_bit++) {
            // surf.create() uses calloc so all pixels get initialized to 0
            // i.e. we can OR bits on instead of requiring explicit setting
            if (picByte & (0x80 >> pixel_bit)) {
              pixels[((lump_i * linesPerLump) + line_i) * width + byte_i * 8 + pixel_bit] |= planed_bit;
            }
          }
          rle1BitBuf <<= 1;
          if (--rle1BitCount == 0) {
            if (rle2BitCount == 0) {
              if (rle2 >= dataEnd) {
                warning("rle2 overrun");
                pal.clear();
                surf.free();
                return false;
              }
              rle2BitBuf = *rle2++;
              rle2BitCount = 8;
            }
            if (rle2BitBuf & 0x80) {
              if (rle1 >= dataEnd) {
                warning("rle1 overrun");
                pal.clear();
                surf.free();
                return false;
              }
              prevRle1BitBuf = rle1BitBuf = *rle1++;
              rle1BitCount = 8;
            }
            else {
              rle1BitBuf = prevRle1BitBuf;
              rle1BitCount = 8;
            }
            rle2BitBuf <<= 1;
            rle2BitCount--;
          }

        }
      }
    }
  }
  return true;
}

bool SpriteBank::load(Common::SeekableReadStream *fromStream, Common::Array<Sprite>& outSprites, Graphics::Palette& outPalette) {
  if (!fromStream) {
    return false;
  }
  uint32 magic = fromStream->readUint32BE();
  if (magic != MKTAG('A', 'm', 'S', 'p') && magic != MKTAG('A', 'm', 'I', 'c')) {
    warning("file signature not found");
    return false;
  }
  uint16 count = fromStream->readUint16BE();
  outSprites.resize(count);
  for (uint16 sprite_i = 0; sprite_i < count; sprite_i++) {
    uint16 widthWords = fromStream->readUint16BE();
    uint16 height = fromStream->readUint16BE();
    uint16 bitplanes = fromStream->readUint16BE();
    if (bitplanes < 1 || bitplanes > 5 || widthWords > 16 || height > 1024) {
      if (bitplanes == 0 && widthWords == 0 && height == 0) {
        fromStream->skip(4);
        continue;
      }
      warning("sprite %i invalid (width: %d height %d bitplanes: %d)", sprite_i, widthWords*16, height, bitplanes);      
      outSprites.clear();
      return false;
    }
    uint32 width = widthWords * 16;
    outSprites[sprite_i].surf.create(widthWords * 16, height, Graphics::PixelFormat::createFormatCLUT8());
    outSprites[sprite_i].hotspotX = fromStream->readSint16BE();
    outSprites[sprite_i].hotspotY = fromStream->readSint16BE();
    byte* pixels = (byte*)outSprites[sprite_i].surf.getPixels();
    for (uint16 bitplane_i = 0; bitplane_i < bitplanes; bitplane_i++) {
      const byte planed_bit = 1 << bitplane_i;
      for (uint16 y = 0; y < height; y++) {
        for (uint16 xw = 0; xw < widthWords; xw++) {
          const uint16 chunk = fromStream->readUint16BE();
          for (uint16 xi = 0; xi < 16; xi++) {
            if (chunk & (0x8000 >> xi)) {
              pixels[y * width + xw * 16 + xi] |= planed_bit;
            }
          }
        }
      }
    }
  }
  outPalette.resize(32, false);
  for (uint i = 0; i < 32; i++) {
    byte r,g,b;
    amigaColor(fromStream->readUint16BE(), r, g, b);
    outPalette.set(i, r, g, b);
  }
  if (fromStream->eos() || fromStream->err()) {
    warning("stream error");
    outPalette.clear();
    outSprites.clear();
    return false;
  }
  return true;
}

}
