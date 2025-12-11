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

#include "grac/grac.h"
#include "grac/game.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/path.h"
#include "common/memstream.h"
#include "common/endian.h"

namespace Grac {

GracGame* g_game;

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

bool GracGame::ScriptBank::read(Common::SeekableReadStream* fromStream, uint entryPointCount, uint commentCount) {
  entryPoints.resize(entryPointCount);
  for (uint i = 0; i < entryPointCount; i++) {
    entryPoints[i] = fromStream->readSint16BE();
  }
  uint32 rawSize = fromStream->readUint32BE();
  if (rawSize < 4 || rawSize >= 10*1024*1024) {
    warning("Invalid script block size: %d", rawSize);
    return false;
  }
  uint32 dataSize = rawSize - 4;
  byte *newData = new byte[dataSize];
  if (fromStream->read(newData, dataSize) != dataSize) {
    delete[] newData;
    warning("Failed to read script block");
    return false;
  }
  comments.resize(commentCount);
  for (uint i = 0; i < commentCount; i++) {
    if (!readTerminatedString(fromStream, comments[i])) {
      delete[] newData;
      warning("Failed to read comments");
      return false;
    }
  }
  delete[] bytecode;
  bytecode = newData;
  size = dataSize;
  return true;
}

GracGame::GracGame(const Common::Path& path) {
  g_game = this;

  Common::SeekableReadStream* mainStream = nullptr;

  auto fsnode = Common::FSNode(path);
  if (!fsnode.exists()) {
    error("Path does not exist: %s", path.toString().c_str());
  }
  else if (fsnode.isDirectory()) {
    Common::FSList files;
    if (!fsnode.getChildren(files, Common::FSNode::kListFilesOnly)) {
      error("Unable to get file list");
    }

    for (const Common::FSNode &file : files) {
      Common::String filePath = file.getName();
      filePath.toLowercase();
      if (filePath.matchString("*.info")) continue;
      if (filePath.matchString("grac*")) {
        if (
          filePath.equals("gracplayer2")
          || filePath.equals("gracinstall")
          || filePath.equals("gracplayer")
          || filePath.equals("grac.cont")
          || filePath.equals("grac.inv")
          || filePath.equals("grac.song")
          || filePath.matchString("grac *.picture")
          || filePath.matchString("grac *.character")
          || filePath.matchString("grac *.closeup")
          || filePath.matchString("grac *.object")
          || filePath.matchString("grac *.picture")
          || filePath.matchString("grac *.room")
          || filePath.matchString("grac *.sample")
        ) {
          continue;
        }
      }
      warning("Trying %s", filePath.c_str());
      mainStream = unpack(file.createReadStream());
      if (mainStream) {
        break;
      }
    }
  }
  else if (fsnode.isReadable()) {
    mainStream = unpack(fsnode.createReadStream());
  }
  else {
    error("Path not a folder or a readable file: %s", path.toString().c_str());
  }

  if (!mainStream) {
    error("No main stream found");
  }

  uint sharedResourceCount = 100;
  uint roomCount = sharedResourceCount;
  uint closeupCount = sharedResourceCount;
  uint characterCount = sharedResourceCount;
  uint objectBankCount = sharedResourceCount;
  uint pictureCount = sharedResourceCount;
  uint sampleCount = sharedResourceCount;
  uint animCount = sharedResourceCount;
  uint objectBankCharacterCount = 5;
  uint verbCount = 10;
  uint scriptCommentCount = 101;
  uint stringCount = 1000;
  uint characterScriptCount = 51;

  _controlsDataLength = mainStream->readUint32BE();
  _inventoryDataLength = mainStream->readUint32BE();
  _rooms.resize(roomCount);
  _closeups.resize(closeupCount);
  _characters.resize(characterCount);
  _objectBanks.resize(objectBankCount);
  _pictures.resize(pictureCount);
  _samples.resize(sampleCount);
  _anims.resize(animCount);
  for (uint i = 0; i < roomCount; i++) {
    _rooms[i].dataLength = mainStream->readUint32BE();
  }
  for (uint i = 0; i < closeupCount; i++) {
    _closeups[i].dataLength = mainStream->readUint32BE();
  }
  _inventoryDevIndex = mainStream->readSByte();
  _controlsDevIndex = mainStream->readSByte();
  _controlsPictureIndex = mainStream->readSByte();
  _controlsObjectBankIndex = mainStream->readSByte();
  _songDevIndex = mainStream->readSByte(); // -1 if not present
  _startRoom = mainStream->readSByte();
  _startChar = mainStream->readSByte();
  _startPoint = mainStream->readSByte();
  _fontSize1 = mainStream->readSByte();
  _fontSize2 = mainStream->readSByte();

  for (uint i = 0; i < sharedResourceCount; i++) {
    _characters[i].devIndex = mainStream->readSByte();
    _characters[i].speechColor = mainStream->readSByte();

    if (i == 99) {
      _savesDevIndex = _characters[i].devIndex;
    }

    _objectBanks[i].devIndex = mainStream->readSByte();
    for (uint j = 0; j < objectBankCharacterCount; j++) {
      _objectBanks[i].characters.push_back(mainStream->readSByte());
    }

    _rooms[i].devIndex = mainStream->readSByte();
    _rooms[i].pictureIndex = mainStream->readSByte();
    _rooms[i].objectBankIndex = mainStream->readSByte();

    _closeups[i].devIndex = mainStream->readSByte();
    _closeups[i].pictureIndex = mainStream->readSByte();
    _closeups[i].objectBankIndex = mainStream->readSByte();

    _pictures[i].devIndex = mainStream->readSByte();
    _anims[i].devIndex = mainStream->readSByte();
    _samples[i].devIndex = mainStream->readSByte();
  }

  for (uint i = 0; i < characterCount; i++) {
    _characters[i].descriptionStringIndex = mainStream->readSint16BE();
    for (uint j = 0; j < verbCount; j++) {
      _characters[i].actionScripts.push_back(mainStream->readSint16BE());
    }
  }

  if (!_characterScripts.read(mainStream, characterScriptCount, scriptCommentCount)) {
    error("Unable to read character scripts");
  }

  _strings.resize(stringCount);
  for (uint i = 0; i < stringCount; i++) {
    if (!readTerminatedString(mainStream, _strings[i])) {
      error("Unable to read strings");
    }
  }

  _devicePaths.resize(sharedResourceCount);
  for (uint i = 0; i < sharedResourceCount; i++) {
    if (!readTerminatedString(mainStream, _devicePaths[i])) {
      error("Unable to read device path");
    }
    if (!readTerminatedString(mainStream, _characters[i].name)) {
      error("Unable to read character name string");
    }
  }

  if (!readTerminatedString(mainStream, _fontName1)) {
    error("Unable to read font name");
  }
  if (!readTerminatedString(mainStream, _fontName2)) {
    error("Unable to read font name");
  }

  for (uint i = 0; i < sharedResourceCount; i++) {
    if (!readTerminatedString(mainStream, _objectBanks[i].name)) {
      error("Unable to read object bank name");
    }
    if (!readTerminatedString(mainStream, _rooms[i].name)) {
      error("Unable to read room name");
    }
    if (!readTerminatedString(mainStream, _closeups[i].name)) {
      error("Unable to read closeup name");
    }
    if (!readTerminatedString(mainStream, _pictures[i].name)) {
      error("Unable to read picture name");
    }
    if (!readTerminatedString(mainStream, _anims[i].name)) {
      error("Unable to read anim name");
    }
    if (!readTerminatedString(mainStream, _samples[i].name)) {
      error("Unable to read sample name");
    }
  }

  if (mainStream->eos()) {
    error("Truncated data");
  }
  if (mainStream->err()) {
    error("Stream error");
  }

  delete mainStream;

}

class Grac2Decompressor {
public:
  Grac2Decompressor(const uint32* packedStart, const uint32* packedEnd, byte* unpackedStart, byte* unpackedEnd) :
    _packedEnd(packedEnd),
    _packedStart(packedStart),
    _packedHead(packedEnd),
    _unpackedEnd(unpackedEnd),
    _unpackedStart(unpackedStart),
    _unpackedHead(unpackedEnd) {
  }
private:
  const uint32* _packedEnd;
  const uint32* _packedStart;
  const uint32* _packedHead;
  byte* _unpackedEnd;
  byte* _unpackedStart;
  byte* _unpackedHead;
  uint32 _bitCount = 0;
  uint32 _bitBuf = 0;
  bool canReadBits(uint n) {
    return (_bitCount + 8 * 4 * (_packedHead - _packedStart)) >= n;
  }
  bool fillBitBuf() {
    if (_packedHead <= _packedStart) {
      return false;
    }
    _bitBuf = *--_packedHead;
    _bitCount = 32;
    return true;
  }
  int readBit() {
    if (_bitCount == 0 && !fillBitBuf()) {
      return -1;
    }
    uint b = _bitBuf & 1;
    _bitBuf >>= 1;
    _bitCount--;
    return b;
  }
  int readBits(int n) {
    int v = 0;
    for (int i = 0; i < n; i++) {
      v = (v << 1) | readBit();
    }
    return v;
  }
  bool writeByte(byte b) {
    if (_unpackedHead <= _unpackedStart) {
      return false;
    }
    *--_unpackedHead = b;
    return true;
  }
  bool copy(uint distance, uint count) {
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
public:
  bool readAll() {
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
};

Common::SeekableReadStream* GracGame::unpack(Common::SeekableReadStream *packedStream) {
  if (!packedStream) {
    warning("Unable to unpack: Input stream is null");
    return nullptr;
  }
  if (packedStream->readUint32BE() != MKTAG('G', 'R', '2', '0')) {
    warning("Unable to unpack: GR20 tag not found");
    delete packedStream;
    return nullptr;
  }
  int64 streamLen = packedStream->size();
  if (streamLen < 16 || streamLen % 4 != 0) {
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
  if (!packedStream->seek(8, SEEK_SET)) {
    warning("Unable to unpack: Failed to seek");
    delete packedStream;
    return nullptr;
  }
  uint32 sum = 0;
  for (int64 pos = 8; pos < streamLen-8; pos += 4) {
    sum ^= packedStream->readUint32BE();
  }
  if (sum != checksum) {
    warning("Unable to unpack: Failed checksum (expected %x, got %x)", checksum, sum);
    delete packedStream;
    return nullptr;
  }
  uint32 packedLength = ((uint32)streamLen - 16) / 4;
  uint32* packed = new uint32[packedLength];
  if (!packedStream->seek(8) || packedStream->read(packed, packedLength*4) != packedLength*4 || packedStream->err()) {
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
  Grac2Decompressor decomp(packed, packed + packedLength, unpacked, unpacked + unpackedLength);
  if (!decomp.readAll()) {
    warning("Unable to unpack: Decompression stream error");
    delete[] packed;
    delete[] unpacked;
    return nullptr;
  }
  delete[] packed;
  return new Common::MemoryReadStream(unpacked, unpackedLength, DisposeAfterUse::YES);
}

GracGame::~GracGame() {
  g_game = nullptr;
}

}
