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

#ifndef GRAC_SCRIPT_H
#define GRAC_SCRIPT_H

#include "common/stream.h"
#include "common/array.h"

namespace Grac {

  struct ScriptInstruction {
    int16 opcode;
    int16 param1;
    int16 param2;
    uint8 param1Type; // 0 in GRAC 1
    uint8 param2Type; // 0 in GRAC 1
    Common::String toString() const;
  };

  struct ScriptBank {
    Common::Array<Common::Array<ScriptInstruction>> scripts;
    Common::Array<Common::String> comments;
    bool read(Common::SeekableReadStream* fromStream, uint scriptCount, uint commentCount, int versionMajor);
private:
    bool readV1(Common::SeekableReadStream* fromStream, uint scriptCount);
    bool readV2(Common::SeekableReadStream* fromStream, uint scriptCount, uint commentCount);
  };

}

#endif
