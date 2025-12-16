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

#include "grac/script.h"
#include "grac/data.h"

namespace Grac {

bool ScriptBank::readV2(Common::SeekableReadStream* fromStream, uint scriptCount, uint commentCount) {
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

bool ScriptBank::readV1(Common::SeekableReadStream* fromStream, uint scriptCount) {

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

bool ScriptBank::read(Common::SeekableReadStream* fromStream, uint scriptCount, uint commentCount, int versionMajor) {
  if (versionMajor >= 2) {
    return readV2(fromStream, scriptCount, commentCount);
  }
  else {
    return readV1(fromStream, scriptCount);
  }
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

Common::String ScriptInstruction::toString() const {
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
    case 16: return "compare flag " + encodeParam(param1, param1Type) + ", " + encodeParam(param2, param2Type);
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
