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

#ifndef GRAC_ROOM_H
#define GRAC_ROOM_H

#include "common/stream.h"
#include "common/rect.h"
#include "grac/script.h"

namespace Grac {

enum ObjectAltitude {
  kAltitudeLow,
  kAltitudeMiddle,
  kAltitudeHigh,
  kAltitudeNone,
};

enum ApproachDirection {
  kApproachRight,
  kApproachLeft,
  kApproachIn,
  kApproachOut,
  kApproachNone,
};

class Room {

public:
  struct Object {
    Common::String name;
    int16 walkZone;
    int16 approach;
    int16 altitude;
    int16 message;
    Common::Array<int16> verbScripts;
    int16 enabledFlag;
    Common::Point goToPoint;
  };
  
  struct BackgroundObject : Object {
    Common::Rect hotspot;
    Common::Point pastePos;
    void read(Common::SeekableReadStream *stream);
  };

  struct ForegroundObject : Object {
    Common::Point spritePos;
    int16 spriteImage;
    int16 inventoryItem;
    void read(Common::SeekableReadStream *stream);
  };

  struct WalkZone {
    Common::Rect rect;
    Common::Array<int16> neighbors;
    int16 perspectiveScale1;
    int16 perspectiveScale2;
    int16 enterScript;
    int16 enabledFlag;
    void read(Common::SeekableReadStream *stream, int versionMajor);
  };

  struct WalkPoint {
    Common::Point point;
    int16 faceDir;
    int16 walkZone;
    void read(Common::SeekableReadStream *stream);
  };

private:
  int _unused;
  Common::Array<BackgroundObject> _backgroundObjects;
  Common::Array<WalkZone> _walkZones;
  Common::Array<WalkPoint> _walkPoints;
  Common::Array<ForegroundObject> _foregroundObjects;
  ScriptBank _scriptBank;
  Common::Array<Common::String> _strings;

public:
  bool read(Common::SeekableReadStream* fromStream, int versionMajor);
  const ScriptBank* getScripts() const { return &_scriptBank; }
  const Common::Array<ForegroundObject>& getForegroundObjects() const { return _foregroundObjects; }
  const Common::Array<BackgroundObject>& geBackgroundObjects() const { return _backgroundObjects; }

};

}

#endif
