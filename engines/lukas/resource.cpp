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

#include "lukas/resource.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"

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

} // End of namespace Lukas
