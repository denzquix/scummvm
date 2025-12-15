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
#include "common/fs.h"
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

bool GracGame::ScriptBank::readV2(Common::SeekableReadStream* fromStream, uint scriptCount, uint commentCount) {
  scripts.resize(scriptCount);
  if (!fromStream->skip(scriptCount * 2)) {
    warning("Failed to skip initial opcode lookup table");
    return false;
  }
  uint32 rawSize = fromStream->readUint32BE();
  if (rawSize < 4 || rawSize >= 10*1024*1024) {
    warning("Invalid script block size: %d", rawSize);
    return false;
  }
  uint32 dataSize = rawSize - 4;
  byte *data = new byte[dataSize];
  if (fromStream->read(data, dataSize) != dataSize) {
    delete[] data;
    warning("Failed to read script block");
    return false;
  }
  const byte *inData = data;
  const byte *dataEnd = data + dataSize;
  for (uint script_i = 0; script_i < scriptCount; script_i++) {
    if ((inData+4) > dataEnd) {
      delete[] data;
      warning("Missing script definition");
      return false;
    }
    if (READ_BE_UINT16(inData) != 0xb620) {
      delete[] data;
      warning("Missing script begin marker");
      return false;
    }
    inData += 2;
    if ((inData+2) > dataEnd) {
      delete[] data;
      warning("Unexpected end of bytecode");
      return false;
    }
    for (; (inData + 8) < dataEnd; inData += 8) {
      ScriptInstruction instr;
      if ((instr.opcode = READ_BE_INT16(inData)) < 1) {
        if ((uint16)instr.opcode == 0xb620) {
          break;
        }
      }
      else {
        instr.param1 = READ_BE_UINT16(inData + 2);
        instr.param2 = READ_BE_UINT16(inData + 4);
        instr.param1Type = inData[6];
        instr.param2Type = inData[7];
        scripts[script_i].push_back(instr);
      }
    }
  }
  delete[] data;
  comments.resize(commentCount);
  for (uint i = 0; i < commentCount; i++) {
    if (!readTerminatedString(fromStream, comments[i])) {
      warning("Failed to read comments");
      return false;
    }
  }
  return true;
}

bool GracGame::ScriptBank::readV1(Common::SeekableReadStream* fromStream, uint scriptCount) {

  if (!fromStream) {
    return false;
  }

  const uint instructionsPerScript = 16;
  
  scripts.resize(scriptCount);

  for (uint script_i = 0; script_i < scriptCount; script_i++) {
    for (uint instr_i = 0; instr_i < instructionsPerScript; instr_i++) {
      ScriptInstruction instr;
      if ((instr.opcode = fromStream->readUint16BE()) < 1) {
        if (!fromStream->skip(4 + 6 * (instructionsPerScript - instr_i - 1))) {
          return false;
        }
        break;
      }
      instr.param1 = fromStream->readUint16BE();
      instr.param2 = fromStream->readUint16BE();
      instr.param1Type = 0;
      instr.param2Type = 0;
      scripts[script_i].push_back(instr);
    }
  }

  if (fromStream->err() || fromStream->eos()) {
    scripts.clear();
    return false;
  }
  
  comments.clear();
  
  return true;
}

GracGame::GracGame(const Common::Path& path, int versionMajor): _versionMajor(versionMajor) {

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
          || filePath.matchString("grac *.anim")
          || filePath.matchString("grac *.room")
          || filePath.matchString("grac *.sample")
        ) {
          continue;
        }
      }
      warning("Trying %s", filePath.c_str());
      if (versionMajor == -1) {
        mainStream = unpack(file.createReadStream(), true);
        if (mainStream) {
          versionMajor = 2;
        }
        else {
          mainStream = unpack(file.createReadStream(), true);
          if (mainStream) {
            versionMajor = 1;
          }
        }
      }
      else {
        mainStream = unpack(file.createReadStream(), _versionMajor >= 2);
      }
      if (mainStream) {
        break;
      }
    }
  }
  else if (fsnode.isReadable()) {
    if (versionMajor == -1) {
      mainStream = unpack(fsnode.createReadStream(), true);
      if (mainStream) {
        versionMajor = 2;
      }
      else {
        mainStream = unpack(fsnode.createReadStream(), true);
        if (mainStream) {
          versionMajor = 1;
        }
      }
    }
    else {
      mainStream = unpack(fsnode.createReadStream(), _versionMajor >= 2);
    }
  }
  else {
    error("Path not a folder or a readable file: %s", path.toString().c_str());
  }

  if (!mainStream) {
    error("No main stream found");
  }

  uint sharedResourceCount = 100;
  uint roomCount = sharedResourceCount;
  uint closeupCount = (versionMajor >= 2) ? sharedResourceCount : 0;
  uint characterCount = sharedResourceCount;
  uint objectBankCount = sharedResourceCount;
  uint pictureCount = sharedResourceCount;
  uint sampleCount = sharedResourceCount;
  uint animCount = (versionMajor >= 2) ? sharedResourceCount : 0;
  uint objectBankCharacterCount = 5;
  uint verbCount = 10;
  uint scriptCommentCount = (versionMajor >= 2) ? 101 : 0;
  uint stringCount = 1000;
  uint inventoryItemCount = 100;
  uint characterScriptCount = 51;
  uint verbScriptCount = verbCount;
  uint inventoryScriptCount = 90;

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
  _startCharacter = mainStream->readSByte();
  _startPoint = mainStream->readSByte();
  _controlFontSize = mainStream->readSByte();
  _speechFontSize = mainStream->readSByte();

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

    if (versionMajor >= 2) {
      _closeups[i].devIndex = mainStream->readSByte();
      _closeups[i].pictureIndex = mainStream->readSByte();
      _closeups[i].objectBankIndex = mainStream->readSByte();
    }

    _pictures[i].devIndex = mainStream->readSByte();

    if (versionMajor >= 2) {
      _anims[i].devIndex = mainStream->readSByte();
    }

    _samples[i].devIndex = mainStream->readSByte();
  }

  for (uint i = 0; i < characterCount; i++) {
    _characters[i].descriptionStringIndex = mainStream->readSint16BE();
    for (uint j = 0; j < verbCount; j++) {
      _characters[i].actionScripts.push_back(mainStream->readSint16BE());
    }
  }

  if (versionMajor >= 2) {
    if (!_characterScripts.readV2(mainStream, characterScriptCount, scriptCommentCount)) {
      error("Unable to read character scripts");
    }
  }
  else {
    if (!_characterScripts.readV1(mainStream, characterScriptCount)) {
      error("Unable to read character scripts");
    }
  }

  if (versionMajor >= 2) {
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

    if (!readTerminatedString(mainStream, _controlFontName)) {
      error("Unable to read font name");
    }
    if (!readTerminatedString(mainStream, _speechFontName)) {
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
  }
  else {
    if (!readTerminatedString(mainStream, _controlFontName)) {
      error("Unable to read font name");
    }
    warning("control font: \"%s\"", _controlFontName.c_str());
    if (!readTerminatedString(mainStream, _speechFontName)) {
      error("Unable to read font name");
    }
    warning("speech font: \"%s\"", _speechFontName.c_str());

    _devicePaths.resize(sharedResourceCount);
    for (uint i = 0; i < sharedResourceCount; i++) {
      if (!readTerminatedString(mainStream, _devicePaths[i])) {
        error("Unable to read device path");
      }
      if (!readTerminatedString(mainStream, _characters[i].name)) {
        error("Unable to read character name string");
      }
      if (!readTerminatedString(mainStream, _objectBanks[i].name)) {
        error("Unable to read object bank name");
      }
      if (!readTerminatedString(mainStream, _rooms[i].name)) {
        error("Unable to read room name");
      }
      if (!readTerminatedString(mainStream, _pictures[i].name)) {
        error("Unable to read picture name");
      }
      if (!readTerminatedString(mainStream, _samples[i].name)) {
        error("Unable to read sample name");
      }
    }

    _strings.resize(stringCount);
    for (uint i = 0; i < stringCount; i++) {
      if (!readTerminatedString(mainStream, _strings[i])) {
        error("Unable to read strings");
      }
    }
  }

  if (mainStream->eos()) {
    error("Truncated data");
  }
  if (mainStream->err()) {
    error("Stream error");
  }

  delete mainStream;

  Common::FSNode fsNode;
  if (!findFile("GRAC.cont", fsNode)) {
    error("GRAC.cont file not found");
  }
  Common::SeekableReadStream* controlsStream = fsNode.createReadStream();
  if (!controlsStream) {
    error("Unable to open GRAC.cont file");
  }
  controlsStream = unpack(controlsStream, false);
  if (!controlsStream) {
    error("Failed to unpack GRAC.cont file");
  }
  _controlsX = controlsStream->readSint16BE();
  _controlsY = controlsStream->readSint16BE();
  _controlsWidth = controlsStream->readSint16BE();
  _controlsHeight = controlsStream->readSint16BE();
  _roomViewWidth = controlsStream->readSint16BE();
  _roomViewHeight = controlsStream->readSint16BE();
  _messageBoxX1 = controlsStream->readSint16BE();
  _messageBoxY1 = controlsStream->readSint16BE();
  _messageBoxX2 = controlsStream->readSint16BE();
  _messageBoxY2 = controlsStream->readSint16BE();
  _verbLineX = controlsStream->readSint16BE();
  _verbLineY = controlsStream->readSint16BE();
  (void)controlsStream->readSint16BE(); // unused
  (void)controlsStream->readSint16BE(); // unused
  _controlTextFgColor = controlsStream->readSint16BE();
  _controlTextBgColor = controlsStream->readSint16BE();

  _verbs.resize(verbCount);
  for (uint verb_i = 0; verb_i < verbCount; verb_i++) {
    _verbs[verb_i].downImage = controlsStream->readSint16BE();
    _verbs[verb_i].zoneRect.left = controlsStream->readSint16BE();
    _verbs[verb_i].zoneRect.top = controlsStream->readSint16BE();
    _verbs[verb_i].zoneRect.right = controlsStream->readSint16BE();
    _verbs[verb_i].zoneRect.bottom = controlsStream->readSint16BE();
    _verbs[verb_i].imagePoint.x = controlsStream->readSint16BE();
    _verbs[verb_i].imagePoint.y = controlsStream->readSint16BE();
    _verbs[verb_i].upImage = controlsStream->readSint16BE();
    (void)controlsStream->readSint16BE(); // unused
    _verbs[verb_i].defaultMessage = controlsStream->readSint16BE();
    _verbs[verb_i].type = controlsStream->readSint16BE();
    _verbs[verb_i].messageDisplayMode = controlsStream->readSint16BE();
  }
  if (_versionMajor >= 2) {
    if (!_verbScripts.readV2(controlsStream, verbScriptCount, scriptCommentCount)) {
      error("Failed to load verb scripts");
    }
  }
  else {
    if (!_verbScripts.readV1(controlsStream, verbScriptCount)) {
      error("Failed to load verb scripts");
    }
  }
  for (uint verb_i = 0; verb_i < verbCount; verb_i++) {
    if (!readTerminatedString(controlsStream, _verbs[verb_i].text)) {
      error("Failed to read verb");
    }
    if (!readTerminatedString(controlsStream, _verbs[verb_i].preposition)) {
      error("Failed to read preposition");
    }
  }

  if (controlsStream->eos()) {
    error("Truncated data");
  }
  if (controlsStream->err()) {
    error("Stream error");
  }

  delete controlsStream;

  if (!findFile("GRAC.inv", fsNode)) {
    error("GRAC.inv file not found");
  }
  Common::SeekableReadStream* invStream = fsNode.createReadStream();
  if (!invStream) {
    error("Unable to open GRAC.inv file");
  }
  invStream = unpack(invStream, false);
  if (!invStream) {
    error("Failed to unpack GRAC.inv file");
  }
  _inventoryItems.resize(inventoryItemCount);
  for (uint inv_i = 0; inv_i < inventoryItemCount; inv_i++) {
    _inventoryItems[inv_i].defaultMessage = invStream->readSint16BE();
    _inventoryItems[inv_i].scripts.resize(verbCount);
    for (uint verb_i = 0; verb_i < verbCount; verb_i++) {
      _inventoryItems[inv_i].scripts[verb_i] = invStream->readSint16BE();
    }
  }

  if (_versionMajor >= 2) {
    if (!_inventoryScripts.readV2(invStream, inventoryScriptCount, scriptCommentCount)) {
      error("Failed to load verb scripts");
    }
  }
  else {
    if (!_inventoryScripts.readV1(invStream, inventoryScriptCount)) {
      error("Failed to load verb scripts");
    }
  }
  for (uint inv_i = 0; inv_i < inventoryItemCount; inv_i++) {
    if (!readTerminatedString(invStream, _inventoryItems[inv_i].name)) {
      error("Unable to read inventory item name");
    }
  }

  if (invStream->eos()) {
    error("Truncated data");
  }
  if (invStream->err()) {
    error("Stream error");
  }

  delete invStream;

  _controlFont = loadFont(_controlFontName, _controlFontSize);
  _speechFont = loadFont(_speechFontName, _speechFontSize);

  g_game = this;
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

Common::SeekableReadStream* GracGame::unpack(Common::SeekableReadStream *packedStream, bool isGrac2MainFile) {
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
  delete _speechFont;
  delete _controlFont;
}

bool GracGame::findFile(const Common::String& name, Common::FSNode& outNode) {
  Common::FSNode node(ConfMan.getPath("path"));
  Common::FSNode direct = node.getChild(name);
  if (direct.exists()) {
    outNode = direct;
    return true;
  }
  Common::FSNode dataFolder = node.getChild("data");
  if (dataFolder.isDirectory()) {
    Common::FSNode inData = dataFolder.getChild(name);
    if (inData.exists()) {
      outNode = inData;
      return true;
    }
  }
  return false;
}

const Graphics::AmigaFont* GracGame::loadFont(const Common::String& name, int size) {
  Common::FSNode node(ConfMan.getPath("path"));
  if (node.exists() && node.isDirectory()) {
    node = node.getChild("fonts");
    if (node.exists() && node.isDirectory()) {
      node = node.getChild(name);
      if (node.exists() && node.isDirectory()) {
        if (size == -1) {
          Common::FSList files;
          if (node.getChildren(files, Common::FSNode::kListFilesOnly) && !files.empty()) {
            node = files[0];
            Common::ScopedPtr<Common::SeekableReadStream> stream(node.createReadStream());
            if (stream.get() != nullptr) {
              return new Graphics::AmigaFont(stream.get());
            }
          }
        }
        else {
          node = node.getChild(Common::String::format("%d", size));
          if (node.exists() && !node.isDirectory() && node.isReadable()) {
            Common::ScopedPtr<Common::SeekableReadStream> stream(node.createReadStream());
            return new Graphics::AmigaFont(stream.get());
          }
        }
      }
    }
  }
  return new Graphics::AmigaFont(); // fall back to default
}

static Common::String encodeParam(int16 paramValue, uint8 modifier) {
  switch (modifier) {
    case 0: return Common::String::format("%d", paramValue);
    case 1: return Common::String::format("#%d", paramValue);
    case 2: return "room";
    case 3: return "item";
    case 4: return "pc";
    case 5: return "entrance";
    case 6: return "gtime";
    case 7: return "rtime";
    case 8: return "current";
    case 9: return "string";
    case 10: return "height";
    default: return Common::String::format("%d [unknown modifier: %d]", paramValue, modifier);
  }
}

static Common::String encodeComparator(int16 paramValue) {
  switch (paramValue) {
    case 1: return ">";
    case 2: return ">=";
    case 3: return "=";
    case 4: return "<=";
    case 5: return "<";
    default: return Common::String::format("[unknown comparator %d]", paramValue);
  }
}

Common::String GracGame::ScriptInstruction::toString() const {
  switch(opcode) {
    case 1: return "bell";
    case 2: return "execute";
    case 19: return "end if";
    case 27: return "else";
    case 31: return "end select";
    case 45: return "no default";
    case 46: return "freeze";
    case 47: return "unfreeze";
    case 51: return "fade in";
    case 55: return "timer off";
    case 65: return "hide";
    case 66: return "show";
    case 71: return "music stop";
    case 72: return "restart";
    case 75: return "picture off";
    case 77: return "quit";
    case 79: return "limbo";
    case 83: return "scroll off";
    case 84: return "scroll on";
    case 89: return "exit close up";
    case 93: return "cycle off";
    case 94: return "save off";
    case 96: return "save on";
    case 98: return "walk off";
    case 99: return "walk on";
    case 101: return "*comment";
    case 104: return "pause off";
    case 108: return "end";

    case 3: return "go " + encodeParam(param1, param1Type);
    case 4: return "wait stop " + encodeParam(param1, param1Type);
    case 5: return "print " + encodeParam(param1, param1Type);
    case 9: return "stop " + encodeParam(param1, param1Type);
    case 14: return "wait " + encodeParam(param1, param1Type);
    case 29: return "script " + encodeParam(param1, param1Type);
    case 41: return "next " + encodeParam(param1, param1Type);
    case 42: return "clear string " + encodeParam(param1, param1Type);
    case 48: return "load palette " + encodeParam(param1, param1Type);
    case 49: return "fade " + encodeParam(param1, param1Type);
    case 61: return "load sample " + encodeParam(param1, param1Type);
    case 64: return "erase sample " + encodeParam(param1, param1Type);
    case 67: return "hide character " + encodeParam(param1, param1Type);
    case 74: return "show picture " + encodeParam(param1, param1Type);
    case 76: return "wait click " + encodeParam(param1, param1Type); // timeout
    case 81: return "fade picture " + encodeParam(param1, param1Type);
    case 82: return "toggle flag " + encodeParam(param1, param1Type);
    case 86: return "switch " + encodeParam(param1, param1Type);
    case 87: return "dcontrol palette " + encodeParam(param1, param1Type);
    case 88: return "fade control " + encodeParam(param1, param1Type);
    case 90: return "close up " + encodeParam(param1, param1Type);
    case 95: return "verb off " + encodeParam(param1, param1Type);
    case 97: return "verb on " + encodeParam(param1, param1Type);
    case 102: return "pause " + encodeParam(param1, param1Type); // frames
    case 106: return "set mark " + encodeParam(param1, param1Type);
    case 107: return "goto mark " + encodeParam(param1, param1Type);
    case 110: return "static " + encodeParam(param1, param1Type);
    case 111: return "scale " + encodeParam(param1, param1Type); // percent

    case 6: return "reach " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 7: return "take " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 8: return "paste " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 10: return "say " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 11: return "load room " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 12: return "character frame " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 13: return "object frame " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 15: return "face " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 16: return "compare flag" + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 17: return "compare value " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 18: return "if " + encodeComparator(param1) + ", " + encodeParam(param2, param2Type);
    case 20: return "compare item " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 21: return "add item" + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 22: return "drop item " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 23: return "set flag " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 24: return "link " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 25: return "choice " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 26: return "choose " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 28: return "else if " + encodeComparator(param1) + ", " + encodeParam(param2, param2Type);
    case 30: return "select " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 32: return "random " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 33: return "add " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 34: return "add flag " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 35: return "subtract flag " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 36: return "set string " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 37: return "add string " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 38: return "number to string " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 39: return "flag to string " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 40: return "for " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 43: return "copy flag " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 44: return "amal " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 50: return "timer " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 52: return "compare entry " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 53: return "place character " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 54: return "walk " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 56: return "sound left " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 57: return "sound right " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 58: return "sound centre " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 59: return "sound back " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 60: return "st play " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 62: return "body frame " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 63: return "med play " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 68: return "goto " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 69: return "voice " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 70: return "set voice " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 73: return "character change " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 78: return "play anim " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 80: return "flash " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 85: return "scroll " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 91: return "clear flags " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 92: return "cycle " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 100: return "compare " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 103: return "anim " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 105: return "subtract " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 109: return "perspective " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    case 112: return "position voice " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
    
    default: return Common::String::format("opcode_%04X", (uint16)opcode) + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
  }
}

}
