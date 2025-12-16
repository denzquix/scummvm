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
#include "grac/data.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/path.h"
#include "common/memstream.h"
#include "common/endian.h"

namespace Grac {

GracGame* g_game;

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
        mainStream = decompress(file.createReadStream(), true);
        if (mainStream) {
          versionMajor = 2;
        }
        else {
          mainStream = decompress(file.createReadStream(), true);
          if (mainStream) {
            versionMajor = 1;
          }
        }
      }
      else {
        mainStream = decompress(file.createReadStream(), _versionMajor >= 2);
      }
      if (mainStream) {
        break;
      }
    }
  }
  else if (fsnode.isReadable()) {
    if (versionMajor == -1) {
      mainStream = decompress(fsnode.createReadStream(), true);
      if (mainStream) {
        versionMajor = 2;
      }
      else {
        mainStream = decompress(fsnode.createReadStream(), true);
        if (mainStream) {
          versionMajor = 1;
        }
      }
    }
    else {
      mainStream = decompress(fsnode.createReadStream(), _versionMajor >= 2);
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

  if (!_characterScripts.read(mainStream, characterScriptCount, scriptCommentCount, versionMajor)) {
    error("Unable to read character scripts");
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
  controlsStream = decompress(controlsStream, false);
  if (!controlsStream) {
    error("Failed to unpack GRAC.cont file");
  }
  _controlsPos.x = controlsStream->readSint16BE();
  _controlsPos.y = controlsStream->readSint16BE();
  int roomViewWidth = controlsStream->readSint16BE();
  int roomViewHeight = controlsStream->readSint16BE();
  _roomViewRect.left = controlsStream->readSint16BE();
  _roomViewRect.top = controlsStream->readSint16BE();
  _roomViewRect.setWidth(roomViewWidth);
  _roomViewRect.setHeight(roomViewHeight);
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
  if (!_verbScripts.read(controlsStream, verbScriptCount, scriptCommentCount, versionMajor)) {
    error("Failed to load verb scripts");
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
  invStream = decompress(invStream, false);
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

  if (!_inventoryScripts.read(invStream, inventoryScriptCount, scriptCommentCount, versionMajor)) {
    error("Failed to load verb scripts");
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

  _inventoryColumnWidth = 0;
  for (uint inv_i = 0; inv_i < inventoryItemCount; inv_i++) {
    uint width = _controlFont->getStringWidth(_inventoryItems[inv_i].name);
    if (width > _inventoryColumnWidth) _inventoryColumnWidth = width;
  }

  g_game = this;
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

}
