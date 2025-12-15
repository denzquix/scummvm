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

#ifndef GRAC_GAME_H
#define GRAC_GAME_H

#include "common/fs.h"
#include "common/path.h"
#include "common/array.h"
#include "common/stream.h"
#include "graphics/fonts/amigafont.h"
#include "grac/grac.h"

namespace Grac {

bool readTerminatedString(Common::SeekableReadStream *stream, Common::String &outString);

class GracGame {

public:
  struct CharacterDef {
    int8 devIndex; // for "GRAC x.character"
    int8 speechColor;
    int16 descriptionStringIndex;
    Common::Array<int16> actionScripts;
    Common::String name;
  };

  struct ObjectBankDef {
    int8 devIndex; // for "GRAC x.object"
    Common::Array<int8> characters;
    Common::String name;
  };

  struct RoomDef {
    uint32 dataLength;
    int8 devIndex; // for "GRAC x.room"
    int8 pictureIndex;
    int8 objectBankIndex;
    Common::String name;
  };

  struct CloseupDef {
    uint32 dataLength;
    int8 devIndex; // for "GRAC x.closeup"
    int8 pictureIndex;
    int8 objectBankIndex;
    Common::String name;
  };

  struct PictureDef {
    int8 devIndex; // for "GRAC x.picture"
    Common::String name;
  };

  struct AnimationDef {
    int8 devIndex; // for "GRAC x.anim"
    Common::String name;
  };

  struct SampleDef {
    int8 devIndex; // for "GRAC x.sample"
    Common::String name;
  };

  struct ScriptInstruction {
    int16 opcode;
    int16 param1;
    int16 param2;
    uint8 param1Type; // 0 in GRAC 1
    uint8 param2Type; // 0 in GRAC 1
    Common::String toString() const;
  };

  enum VerbType {
    kVerbAny = 1,
    kVerbInv = 2,
    kVerbRoom = 3,
    kVerbInvAny = 4,
    kVerbInvInv = 5,
    kVerbInvRoom = 6,
    kVerbQuick = 7,
  };

  enum MessageDisplayMode {
    kMessageDisplayPrint = 0,
    kMessageDisplaySay = 1,
    kMessageDisplayNone = 2,
  };

  struct VerbDef {
    int16 downImage;
    int16 upImage;
    Common::Point imagePoint;
    Common::Rect zoneRect;
    Common::String text;
    Common::String preposition;
    int16 type;
    int16 messageDisplayMode;
    int16 defaultMessage;
  };

  struct ScriptBank {
    bool readV1(Common::SeekableReadStream* fromStream, uint scriptCount);
    bool readV2(Common::SeekableReadStream* fromStream, uint scriptCount, uint commentCount);

    Common::Array<Common::Array<ScriptInstruction>> scripts;
    Common::Array<Common::String> comments;
  };

private:
  Common::SeekableReadStream* unpack(Common::SeekableReadStream *packedStream, bool isGrac2MainFile);  
  int _versionMajor;
  uint32 _controlsDataLength;
  uint32 _inventoryDataLength;
  int8 _inventoryDevIndex;
  int8 _controlsDevIndex;
  int8 _controlsPictureIndex;
  int8 _controlsObjectBankIndex;
  int8 _songDevIndex;
  int8 _startRoom;
  int8 _startCharacter;
  int8 _startPoint;
  int8 _controlFontSize;
  int8 _speechFontSize;
  Common::Array<CharacterDef> _characters;
  int8 _savesDevIndex;
  Common::Array<ObjectBankDef> _objectBanks;
  Common::Array<RoomDef> _rooms;
  Common::Array<CloseupDef> _closeups;
  Common::Array<PictureDef> _pictures;
  Common::Array<AnimationDef> _anims;
  Common::Array<SampleDef> _samples;
  uint _roomScriptsStart = 0;
  uint _roomScriptsEnd = 50;
  uint _controlScriptsStart = 50;
  uint _controlScriptsEnd = 60;
  uint _inventoryScriptsStart = 60;
  uint _inventoryScriptsEnd = 150;
  uint _characterScriptsStart = 150;
  uint _characterScriptsEnd = 201;
  uint _closeupScriptsStart = 201;
  uint _closeupScriptsEnd = 226;
  ScriptBank _roomScripts;
  ScriptBank _verbScripts;
  ScriptBank _inventoryScripts;
  ScriptBank _characterScripts;
  ScriptBank _closeupScripts;
  Common::Array<Common::String> _strings;
  Common::Array<Common::String> _devicePaths;
  Common::String _controlFontName;
  Common::String _speechFontName;
  const Graphics::AmigaFont* _speechFont;
  const Graphics::AmigaFont* _controlFont;
  const Graphics::AmigaFont* loadFont(const Common::String& name, int size);
  int _controlsX;
  int _controlsY;
  int _controlsWidth;
  int _controlsHeight;
  int _roomViewWidth;
  int _roomViewHeight;
  int _messageBoxX1;
  int _messageBoxY1;
  int _messageBoxX2;
  int _messageBoxY2;
  int _verbLineX;
  int _verbLineY;
  int _controlTextFgColor;
  int _controlTextBgColor;
  Common::Array<VerbDef> _verbs;

public:
  GracGame(const Common::Path& path, int versionMajor);
  ~GracGame();

  const Common::Array<RoomDef>& getRooms() const { return _rooms; }
  int8 getStartRoom() const { return _startRoom; }
  bool findFile(const Common::String& name, Common::FSNode& outNode);
  int8 getStartCharacter() const {
    int8 charIndex = _objectBanks[_rooms[_startRoom].objectBankIndex].characters[_startCharacter];
    if (charIndex == -1) charIndex = 0;
    return charIndex;
  }
  const Graphics::AmigaFont* getSpeechFont() const { return _speechFont; }
  const Graphics::AmigaFont* getControlFont() const { return _controlFont; }
  int getCharacterSpeechColor(int charIndex) const { return charIndex < 0 || (uint)charIndex >= _characters.size() ? -1 : _characters[charIndex].speechColor; }
  const ScriptBank* getCharacterScripts() const { return &_characterScripts; }

};

extern GracGame *g_game;

}

#endif
