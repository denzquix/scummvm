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

#include "lukas/dialogue.h"
#include "lukas/resource.h"
#include "lukas/room.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

namespace Lukas {

ResourceManager::ResourceManager() {
}

ResourceManager::ResourceManager(const Common::Path &gameDataPath) {
	ensureResourceRoot(gameDataPath);
}

Common::SeekableReadStream *ResourceManager::loadResourceFile(const Common::String &resourceName) const {
	Common::String normalizedName = normalizeName(resourceName);

	if (!_resourceRoot.exists()) {
		warning("Resource root not configured");
		return nullptr;
	}

	Common::FSNode resourceFile = _resourceRoot.getChild(normalizedName);
	if (!resourceFile.exists()) {
		warning("Resource '%s' not found", normalizedName.c_str());
		return nullptr;
	}

  Common::SeekableReadStream* stream = resourceFile.createReadStream();

  if (!stream) {
		warning("Resource '%s' not found", normalizedName.c_str());
    return nullptr;
  }

  return toResourceStream(stream);
}

Common::SeekableReadStream *ResourceManager::loadResource(const Common::Path &p) const {
  if (p.empty()) {
    return nullptr;
  }
  const Common::Path parent = p.getParent().removeTrailingSeparators();
  warning("[Parent of %s is %s]", p.toString().c_str(), parent.toString().c_str());
  if (parent.empty()) {
    warning("Root - loading resource file");
    return loadResourceFile(p.toString());
  }
  Common::SeekableReadStream *parentStream = loadResource(parent);
  if (!parentStream) {
    warning("Parent stream is null %s", parent.toString().c_str());
    return nullptr;
  }
  auto subres = getSubresourceSpans(parentStream);
  if (subres.empty()) {
    warning("No subres for %s", parent.toString().c_str());
    delete parentStream;
    return nullptr;
  }
  const Common::String indexStr = p.getLastComponent().removeTrailingSeparators().toString();
  // reject leading zeros to only allow one canonical form
  if (indexStr.size() > 1 && indexStr[0] == '0') {
    warning("Leading zero(s): %s", p.toString().c_str());
    delete parentStream;
    return nullptr;
  }
  uint64 index64 = indexStr.asUint64();
  if (index64 > UINT32_MAX) {
    warning("Out of uint32 range: %s", p.toString().c_str());
    delete parentStream;
    return nullptr;
  }
  uint32 index = (uint32)index64;
  if (index == 0) {
    if (indexStr.empty()) {
      warning("Empty name: %s", p.toString().c_str());
      delete parentStream;
      return nullptr;
    }
    for (uint i = 0; i < indexStr.size(); i++) {
      if (!Common::isDigit(indexStr[i])) {
        warning("Invalid index: %s", p.toString().c_str());
        delete parentStream;
        return nullptr;
      }
    }
  }
  if (index >= subres.size()) {
    warning("Subresource out of range: %s", p.toString().c_str());
    delete parentStream;
    return nullptr;
  }
  warning("Getting subresource %d-%d of %s", subres[index].first, subres[index].second, parent.toString().c_str());
  return getSubresource(parentStream, subres[index].first, subres[index].second, DisposeAfterUse::YES);
}

Common::String ResourceManager::normalizeName(const Common::String &resourceName) const {
	if (resourceName.hasSuffixIgnoreCase(".dat")) {
		return resourceName;
  }

	Common::String normalized = resourceName;
	normalized += ".DAT";
	return normalized;
}

bool ResourceManager::ensureResourceRoot(const Common::Path &gameDataPath) {
	Common::FSNode gameRoot = Common::FSNode(gameDataPath);
	if (!gameRoot.exists()) {
		warning("Invalid game data path: %s", gameDataPath.toString().c_str());
		return false;
	}

	Common::FSNode resourceDir = gameRoot.getChild("RESOURCE");
	if (!resourceDir.isDirectory()) {
		warning("RESOURCE directory not found at %s", gameDataPath.toString().c_str());
		return false;
	}

	_resourceRoot = resourceDir;
	return true;
}

bool ResourceManager::decompressRibInPlace(byte *data, uint32 compressedSize, uint32 fullSize) const {
  if (compressedSize >= fullSize) {
    warning("RIB decompression failed: final size must be greater than compressed size");
    return false;
  }
  int32 inIdx = compressedSize - 1;
  int32 outIdx = fullSize - 1;
  while (outIdx > inIdx && outIdx >= 0) {
    int32 ax = data[inIdx--];
    int32 bx = (ax >> 4) & 0x0F;

    switch (bx) {
      case 0: {
        int32 cx = (ax & 0x0F) + 4;
        int32 b = data[inIdx--];
        memset(data + outIdx - cx + 1, b, cx);
        outIdx -= cx;
        break;
      }
      case 1: {
        int32 temp = ax & 0x0F;
        int32 b = data[inIdx--];
        int32 cx = ((temp << 8) | b) + 0x14;
        int32 val = data[inIdx--];
        memset(data + outIdx - cx + 1, val, cx);
        outIdx -= cx;
        break;
      }
      case 2: {
        int32 cx = (ax & 0x0F) + 1;
        for (int32 i = 0; i < cx; i++) {
          data[outIdx--] = data[inIdx--];
        }
        break;
      }
      case 3: {
        int32 temp = ax & 0x0F;
        int32 b = data[inIdx--];
        int32 cx = ((temp << 8) | b) + 0x11;
        for (int32 i = 0; i < cx; i++) {
          data[outIdx--] = data[inIdx--];
        }
        break;
      }
      case 4: {
        if (ax == 0x40) {
          int32 ch = data[inIdx--];
          int32 cl = data[inIdx--];
          int32 cx = (ch << 8) | cl;
          for (int32 i = 0; i < cx; i++) {
            data[outIdx--] = data[inIdx--];
          }
        }
        else if (ax == 0x41) {
          int32 bh = data[inIdx--];
          int32 bl = data[inIdx--];
          int32 offset = (bh << 8) | bl;
          int32 ch = data[inIdx--];
          int32 cl = data[inIdx--];
          int32 cx = (ch << 8) | cl;
          int32 tmpIdx = outIdx + offset;
          for (int32 i = 0; i < cx; i++) {
            data[outIdx--] = data[tmpIdx--];
          }
        }
        else if (ax == 0x42) {
          int32 bh = data[inIdx--];
          int32 bl = data[inIdx--];
          int32 offset = (bh << 8) | bl;
          int32 cx = data[inIdx--] + 0x11;
          int32 tmpIdx = outIdx + offset;
          for (int32 i = 0; i < cx; i++) {
            data[outIdx--] = data[tmpIdx--];
          }
        }
        else {
          int32 cx = (ax & 0x0F) + 1;
          int32 bh = data[inIdx--];
          int32 bl = data[inIdx--];
          int32 offset = (bh << 8) | bl;
          int32 tmpIdx = outIdx + offset;
          for (int32 i = 0; i < cx; i++) {
            data[outIdx--] = data[tmpIdx--];
          }
        }
        break;
      }
      case 5: {
        int32 bh = ax & 0x0F;
        int32 bl = data[inIdx--];
        int32 cx = data[inIdx--] + 0xE;
        int32 offset = ((bh << 8) | bl) + 2;
        int32 tmpIdx = outIdx + offset;
        for (int32 i = 0; i < cx; i++) {
          data[outIdx--] = data[tmpIdx--];
        }
        break;
      }
      default: {
        int32 bh = ax & 0x0F;
        int32 cx = ((ax >> 4) & 0x0F) - 2;
        int32 bl = data[inIdx--];
        int32 offset = ((bh << 8) | bl) + 2;
        int32 tmpIdx = outIdx + offset;
        for (int32 i = 0; i < cx; i++) {
          data[outIdx--] = data[tmpIdx--];
        }
        break;
      }
    }
  }

	return true;
}

Common::SeekableReadStream* ResourceManager::toResourceStream(Common::SeekableReadStream* stream) const {
  if (!stream) {
    return nullptr;
  }
  int64 streamLen64 = stream->size();
  if (streamLen64 < 0) {
    warning("Stream length must be known");
    delete stream;
    return nullptr;
  }
  if (streamLen64 > MAX_RESOURCE_SIZE) {
    warning("Max resource length exceeded");
    delete stream;
    return nullptr;
  }
  uint32 streamLen = (uint32)streamLen64;

  // Check for "NULLPTR" or "NULLPTR\r\n"
  if (streamLen == 7 || streamLen == 9) {
    byte nullCheckBuffer[9];
    if (stream->read(nullCheckBuffer, streamLen) != streamLen) {
      delete stream;
      return nullptr;
    }
    
    if ((streamLen == 7 && memcmp(nullCheckBuffer, "NULLPTR", 7) == 0) ||
        (streamLen == 9 && memcmp(nullCheckBuffer, "NULLPTR\r\n", 9) == 0)) {
      delete stream;
      return nullptr;
    }
    
    if (!stream->seek(0) || stream->err()) {
      delete stream;
      return nullptr;
    }
  }

  if (streamLen <= 8) {
    return stream;
  }
  uint32 marker = stream->readUint32BE();
  if (stream->err()) {
    delete stream;
    return nullptr;
  }
  if (marker != MKTAG('R', 'I', 'B', '\0')) {
    if (!stream->seek(0) || stream->err()) {
      delete stream;
      return nullptr;
    }
    return stream;
  }
  uint32 fullSize = stream->readUint32LE();
  if (stream->err()) {
    delete stream;
    return nullptr;
  }
  if (fullSize > MAX_RESOURCE_SIZE) {
    warning("Max resource length exceeded");
    delete stream;
    return nullptr;
  }

  // We already checked that streamLen >= 8
  uint32 compressedSize = streamLen - 8;

  // Allocate output buffer
  byte* buffer = new byte[fullSize];

  // Copy the compressed payload to the start of the buffer
  if (stream->read(buffer, compressedSize) != compressedSize) {
    delete stream;
    delete[] buffer;
    return nullptr;
  }

  if (!decompressRibInPlace(buffer, compressedSize, fullSize)) {
    delete stream;
    delete[] buffer;
    return nullptr;
  }

  delete stream;

  // Hand back a stream that owns the decompressed buffer
  return new Common::MemoryReadStream(buffer, fullSize, DisposeAfterUse::YES);
}

Common::Array<Common::Pair<uint32, uint32>> ResourceManager::getSubresourceSpans(Common::SeekableReadStream *resourceStream) const {
  if (!resourceStream) {
    return Common::Array<Common::Pair<uint32, uint32>>();
  }
  int64 streamLength = resourceStream->size();
  if (streamLength < 12 || streamLength > MAX_RESOURCE_SIZE) {
    return Common::Array<Common::Pair<uint32, uint32>>();
  }
  Common::Array<Common::Pair<uint32, uint32>> spans;
  for (;;) {
    uint32 ptr = resourceStream->readUint32LE();
    if (resourceStream->err() || resourceStream->eos()) {
      return Common::Array<Common::Pair<uint32, uint32>>();
    }
    if (ptr == 0) {
      break;
    }
    spans.push_back(Common::Pair<uint32, uint32>(ptr, 0));
  }
  if (spans.empty()) {
    return spans;
  }
  uint32 minPtr = spans[0].first;
  for (uint32 i = 0; i < spans.size(); i++) {
    if (spans[i].first < minPtr) {
      minPtr = spans[i].first;
    }
  }
  uint32 tableEndOffset = (spans.size() * 2 + 1) * 4;
  if (streamLength < tableEndOffset || minPtr < tableEndOffset) {
    return Common::Array<Common::Pair<uint32, uint32>>();
  }
  // if the content of a resource is "NULLPTR" or "NULLPTR\r\n",
  // replace its bounds with 0-0
  for (uint i = 0; i < spans.size(); i++) {
    uint32 resourceLength = resourceStream->readUint32LE();
    if (resourceLength == 7 || resourceLength == 9) {
      int64 restorePos = resourceStream->pos();
      resourceStream->seek(spans[i].first);
      byte buf[9];
      if (resourceStream->read(buf, resourceLength) != resourceLength) {
        return Common::Array<Common::Pair<uint32, uint32>>();
      }
      if (memcmp(buf, "NULLPTR\r\n", resourceLength) == 0) {
        spans[i].first = spans[i].second = 0;
        resourceStream->seek(restorePos);
        continue;
      }
      resourceStream->seek(restorePos);
    }
    int64 endOffset = (int64)spans[i].first + resourceLength;
    if (endOffset > streamLength) {
      return Common::Array<Common::Pair<uint32, uint32>>();
    }
    spans[i].second = (uint32)endOffset;
  }
  if (resourceStream->err()) {
    return Common::Array<Common::Pair<uint32, uint32>>();
  }
  return spans;
}

Common::SeekableReadStream *ResourceManager::getSubresource(
  Common::SeekableReadStream *resourceStream,
  uint32 startOffset,
  uint32 endOffset,
  DisposeAfterUse::Flag flag
) const {
  if (!resourceStream) {
    return nullptr;
  }
  return toResourceStream(new Common::SeekableSubReadStream(resourceStream, startOffset, endOffset, flag));
}

static Common::U32String readRoomInfoString(Common::SeekableReadStream *resourceStream, uint32 pos, Common::CodePage page) {
  if (pos == 0)
    return Common::U32String();
  if (pos >= resourceStream->size()) {
    warning("room string out of range");
    return Common::U32String();
  }
  resourceStream->seek(pos);
  Common::String raw = resourceStream->readString();
  if (raw.empty()) return Common::U32String();
  auto unicode = raw.decode(page);
  return unicode;
}

static const uint32 ROOM_INFO_HEADER_SIZE = 8;
static const uint32 ROOM_OBJECT_SIZE = 46;

Common::Array<RoomObjectInfo> ResourceManager::getRoomObjectInfo(Common::SeekableReadStream *resourceStream, Common::CodePage page) const {

  // These values appear to always be the same
  if (resourceStream->readUint32LE() != 8) {
    warning("Not a room object array: missing magic number (header)");
    return Common::Array<RoomObjectInfo>();
  }
  if (resourceStream->readUint16LE() != 0) {
    warning("Not a room object array: missing magic number (header)");
    return Common::Array<RoomObjectInfo>();
  }

  uint16 count = resourceStream->readUint16LE();
  if (resourceStream->eos() || resourceStream->err() || (ROOM_INFO_HEADER_SIZE + (ROOM_OBJECT_SIZE * count)) > resourceStream->size()) {
    warning("Invalid stream");
    return Common::Array<RoomObjectInfo>();
  }
  Common::Array<RoomObjectInfo> objects;
  objects.reserve(count);
  for (uint16 i = 0; i < count; i++) {
    RoomObjectInfo obj;
    resourceStream->seek(ROOM_INFO_HEADER_SIZE + i * ROOM_OBJECT_SIZE);

    // These values appear to always be the same
    if (resourceStream->readUint32LE() != 0) {
      warning("Not a room object array: missing magic number (record)");
      return Common::Array<RoomObjectInfo>();
    }
    if (resourceStream->readUint32LE() != 1) {
      warning("Not a room object array: missing magic number (record)");
      return Common::Array<RoomObjectInfo>();
    }

    obj.x = resourceStream->readUint16LE();
    obj.y = resourceStream->readUint16LE();
    obj.width = resourceStream->readUint16LE();
    obj.height = resourceStream->readUint16LE();
    obj.id = resourceStream->readUint16LE();
    uint32 panelNameOffset = resourceStream->readUint32LE();
    uint32 hoverNameOffset = resourceStream->readUint32LE();
    uint32 openTextOffset = resourceStream->readUint32LE();
    uint32 useTextOffset = resourceStream->readUint32LE();
    uint32 lookTextOffset = resourceStream->readUint32LE();
    uint32 closeTextOffset = resourceStream->readUint32LE();
    uint32 takeTextOffset = resourceStream->readUint32LE();
    obj.panelName = readRoomInfoString(resourceStream, panelNameOffset, page);
    obj.hoverName = readRoomInfoString(resourceStream, hoverNameOffset, page);
    obj.openText = readRoomInfoString(resourceStream, openTextOffset, page);
    obj.useText = readRoomInfoString(resourceStream, useTextOffset, page);
    obj.lookText = readRoomInfoString(resourceStream, lookTextOffset, page);
    obj.closeText = readRoomInfoString(resourceStream, closeTextOffset, page);
    obj.takeText = readRoomInfoString(resourceStream, takeTextOffset, page);
    objects.push_back(obj);
  }
  if (resourceStream->err()) {
    warning("Not a room object array: final error");
    objects.clear();
  }
  return objects;
}

static const uint32 DIALOGUE_HEADER_SIZE = 4;
static const uint32 DIALOGUE_ENTRY_SIZE = 8;

Common::Array<DialogueLine> ResourceManager::getDialogue(Common::SeekableReadStream *resourceStream, Common::CodePage page) const {
  // check for "II" signature
  if (resourceStream->readUint16BE() != MKTAG16('I', 'I')) {
    return Common::Array<DialogueLine>();
  }

  uint16 lineCount = resourceStream->readUint16LE();
  if (resourceStream->eos() || resourceStream->err() || (DIALOGUE_HEADER_SIZE + (DIALOGUE_ENTRY_SIZE * lineCount)) > resourceStream->size()) {
    return Common::Array<DialogueLine>();
  }
  Common::Array<DialogueLine> lines;
  lines.reserve(lineCount);
  for (uint16 line_i = 0; line_i < lineCount; line_i++) {
    DialogueLine line;
    resourceStream->seek(DIALOGUE_HEADER_SIZE + line_i * DIALOGUE_ENTRY_SIZE);
    line.speaker = resourceStream->readUint16LE();
    line.flags = resourceStream->readUint16LE();
    uint16 strOffset = resourceStream->readUint16LE();
    uint16 nextOffset = resourceStream->readUint16LE();
    if (strOffset >= resourceStream->size() || nextOffset >= resourceStream->size()) {
      return Common::Array<DialogueLine>();
    }
    if (strOffset != 0) {
      resourceStream->seek(strOffset);
      while (char c = resourceStream->readByte()) {
        // unprintable characters are the first in a two-byte escape code
        // that we map into a range of Private Use Area - note that the
        // second byte may happen to be in printable range if left as-is
        if (c < 0x20 && c != '\r' && c != '\n') {
          line.text += kControlCodeFirst + c;
          line.text += kControlCodeFirst + resourceStream->readByte();
        }
        else {
          line.text += Common::String(c).decode(page);
        }
        if (resourceStream->eos() || resourceStream->err()) {
          return Common::Array<DialogueLine>();
        }
      }
    }
    if (nextOffset != 0) {
      resourceStream->seek(nextOffset);
      uint16 nextCount = resourceStream->readUint16LE();
      if (resourceStream->err() || ((resourceStream->pos() + nextCount * 2) > resourceStream->size())) {
        return Common::Array<DialogueLine>();
      }
      for (uint16 next_i = 0; next_i < nextCount; next_i++) {
        line.nextLines.push_back(resourceStream->readUint16LE());
      }
    }
    lines.push_back(line);
  }
  if (resourceStream->err()) {
    lines.clear();
  }
  return lines;
}

bool ResourceManager::loadPlainPaletteResource(Common::SeekableReadStream *resourceStream) const {
  if (!resourceStream) {
    return false;
  }
  byte palette[Graphics::PALETTE_SIZE];
  uint32 read = resourceStream->read(palette, Graphics::PALETTE_SIZE);
  if (read == 0 || resourceStream->err() || read%3 != 0 || !resourceStream->eos()) {
    return false;
  }
  for (uint32 i = 0; i < read; i++) {
    if (palette[i] > 63){
      return false;
    }
    palette[i] = PALETTE_6BIT_TO_8BIT(palette[i]);
  }
  g_system->getPaletteManager()->setPalette(palette, 0, read/3);
  return true;
}

bool ResourceManager::loadDeltaPaletteResource(Common::SeekableReadStream *resourceStream) const {
  if (!resourceStream) {
    return false;
  }
  byte palette[Graphics::PALETTE_SIZE];
  while (!resourceStream->eos()) {
    byte offset = resourceStream->readByte();
    uint32 len = resourceStream->readByte();
    if (resourceStream->read(palette, len*3) != len*3 || resourceStream->err()) {
      return false;
    }
    for (uint32 i = 0; i < len*3; i++) {
      if (palette[i] > 63){
        return false;
      }
      palette[i] = PALETTE_6BIT_TO_8BIT(palette[i]);
    }
    g_system->getPaletteManager()->setPalette(palette, offset, len);
  }
  return true;
}

static const uint32 MAX_IMAGE_SIZE = 1024 * 1024;

bool ResourceManager::loadPlainImageResource(Common::SeekableReadStream *resourceStream, Graphics::Surface &surface) const {
  uint16 width = resourceStream->readUint16LE();
  uint16 height = resourceStream->readUint16LE();
  uint32 dataSize = width * height;
  if (dataSize == 0 || dataSize > MAX_IMAGE_SIZE) {
    warning("image loading failed: invalid dimensions %dx%d", width, height);
    return false;
  }
  surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
  if (resourceStream->read(surface.getPixels(), dataSize) != dataSize) {
    warning("image loading failed: did not read enough data");
    surface.free();
    return false;
  }
  return true;
}

bool ResourceManager::loadIconResource(Common::SeekableReadStream *resourceStream, Graphics::Surface &surface) const {
  uint16 width = resourceStream->readUint16LE();
  uint16 height = resourceStream->readUint16LE();
  uint16 mode = resourceStream->readUint32LE();
  if (resourceStream->err()) {
    return false;
  }
  uint32 dataSize = width * height;
  if (dataSize == 0 || dataSize > MAX_IMAGE_SIZE) {
    warning("image loading failed: invalid dimensions %dx%d", width, height);
    return false;
  }
  surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
  if (mode == 0 || mode == 2) {
    if (resourceStream->read(surface.getPixels(), dataSize) != dataSize) {
      warning("image loading failed: did not read enough data");
      surface.free();
      return false;
    }
    return true;
  }
  else if (mode == 1) {
    for (uint y = 0; y < height; y++) {
      byte runLength = resourceStream->readByte();
      byte runMode = resourceStream->readByte();
      switch (runMode) {
        default: {
          surface.free();
          warning("image loading failed: unknown line mode %d", runMode);
          return false;
        }
        case 0x00: {
          surface.free();
          warning("image loading failed: premature end of data");
          return false;
        }
        case 0x02: {
          // blank line
          if (runLength != 0) {
            surface.free();
            warning("image loading failed: expected zero data for line mode 2");
            return false;
          }
          break;
        }
        case 0x04: {
          // literal line
          if (resourceStream->read((byte*)surface.getPixels() + (y * width), width) != width) {
            surface.free();
            warning("image loading failed: not enough data for line mode 4");
            return false;
          }
          break;
        }
        case 0x06: case 0x08: {
          if (runLength < 2) {
            surface.free();
            warning("image loading failed: not enough data for line mode %d", runMode);
            return false;
          }
          byte skipCount = resourceStream->readByte();
          byte copyCount = resourceStream->readByte();
          if (runLength != 2+copyCount) {
            surface.free();
            warning("image loading failed: wrong data length for line mode %d", runMode);
            return false;
          }
          if ((skipCount+copyCount) != width) {
            surface.free();
            warning("image loading failed: wrong pixel width for line mode %d", runMode);
            return false;
          }
          uint x = runMode == 8 ? skipCount : 0;
          if (resourceStream->read((byte*)surface.getPixels() + (y * width) + x, copyCount) != copyCount) {
            surface.free();
            warning("image loading failed: not enough data for line mode %d", runMode);
            return false;
          }
          break;
        }
        case 0xa: {
          if (runLength < 3) {
            surface.free();
            warning("image loading failed: not enough data for line mode %x", runMode);
            return false;
          }
          byte skipCount = resourceStream->readByte();
          byte copyCount1 = resourceStream->readByte();
          if (runLength < 2+copyCount1+1) {
            surface.free();
            warning("image loading failed: wrong data length for line mode %x", runMode);
            return false;
          }
          if (resourceStream->read((byte*)surface.getPixels() + (y * width), copyCount1) != copyCount1) {
            surface.free();
            warning("image loading failed: not enough data for line mode %x", runMode);
            return false;
          }
          byte copyCount2 = resourceStream->readByte();
          if (runLength != 2+copyCount1+1+copyCount2) {
            surface.free();
            warning("image loading failed: wrong data length for line mode %x", runMode);
            return false;
          }
          if (resourceStream->read((byte*)surface.getPixels() + (y * width) + copyCount1 + skipCount, copyCount2) != copyCount2) {
            surface.free();
            warning("image loading failed: not enough data for line mode %x", runMode);
            return false;
          }
          if ((copyCount1+skipCount+copyCount2) != width) {
            surface.free();
            warning("image loading failed: wrong pixel width for line mode %x", runMode);
            return false;
          }
          break;
        }
        case 0x0c: {
          if (runLength < 3) {
            surface.free();
            warning("image loading failed: not enough data for line mode %x", runMode);
            return false;
          }
          byte skipCount1 = resourceStream->readByte();
          byte skipCount2 = resourceStream->readByte();
          byte copyCount = resourceStream->readByte();
          if (runLength != 3+copyCount) {
            surface.free();
            warning("image loading failed: wrong data length for line mode %x", runMode);
            return false;
          }
          if ((skipCount1+copyCount+skipCount2) != width) {
            surface.free();
            warning("image loading failed: wrong pixel width for line mode %x", runMode);
            return false;
          }
          if (resourceStream->read((byte*)surface.getPixels() + (y * width) + skipCount1, copyCount) != copyCount) {
            surface.free();
            warning("image loading failed: not enough data for line mode %x", runMode);
            return false;
          }
          break;
        }
        case 0x0e: {
          if (runLength < 4) {
            surface.free();
            warning("image loading failed: not enough data for line mode %x", runMode);
            return false;
          }
          uint16 x = resourceStream->readByte();
          byte entryCount = resourceStream->readByte();
          for (byte i = 0; i < entryCount; i++) {
            byte skipCount = resourceStream->readByte();
            byte copyCount = resourceStream->readByte();
            if (skipCount == 0 && copyCount == 0) {
              surface.free();
              warning("image loading failed: bad pixel data");
              return false;
            }
            if (resourceStream->read((byte*)surface.getPixels() + (y * width) + x, copyCount) != copyCount) {
              surface.free();
              warning("image loading failed: not enough data for line mode %x", runMode);
              return false;
            }
            x += copyCount + skipCount;
          }
          if (x != width) {
            surface.free();
            warning("image loading failed: wrong pixel width for line mode %x", runMode);
            return false;
          }
          break;
        }
      }
    }
    if (resourceStream->readByte() != 0 || resourceStream->readByte() != 0) {
      surface.free();
      warning("image loading failed: no end marker");
      return false;
    }
    if (resourceStream->err()) {
      surface.free();
      warning("image loading failed: stream error");
      return false;
    }
    return true;
  }
  else {
    surface.free();
    warning("Unknown icon mode: %d", mode);
    return false;
  }
}

bool ResourceManager::loadCursorResource(Common::SeekableReadStream *resourceStream, uint32 keyColor) const {
  if (!resourceStream) {
    return false;
  }
  uint16 width = resourceStream->readUint16LE();
  uint16 height = resourceStream->readUint16LE();
  uint16 hotspotX = resourceStream->readUint16LE();
  uint16 hotspotY = resourceStream->readUint16LE();
  if (resourceStream->err() || width == 0 || height == 0 || width > 256 || height > 256) {
    return false;
  }
  uint32 dataLen = width * height;
  byte *cursorData = new byte[dataLen];
  if (resourceStream->read(cursorData, dataLen) != dataLen) {
    delete[] cursorData;
    return false;
  }
  CursorMan.replaceCursor(cursorData, width, height, hotspotX, hotspotY, keyColor);
  CursorMan.showMouse(true);
  delete[] cursorData;
  return true;
}

} // End of namespace Lukas
