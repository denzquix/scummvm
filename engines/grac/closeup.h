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

#ifndef GRAC_CLOSEUP_H
#define GRAC_CLOSEUP_H

#include "common/rect.h"
#include "common/stream.h"
#include "grac/script.h"

namespace Grac {

class Closeup {

public:
  struct Zone {
    Common::Rect hotspot;
    Common::Point imagePosition;
    int16 upImage, downImage;
    int16 clickScript;

    void read(Common::SeekableReadStream *fromStream);
  };

private:
  ScriptBank _scripts;
  Common::Array<Zone> _zones;

  static const uint kZoneCount = 16;

public:

  bool read(Common::SeekableReadStream *fromStream);
  const ScriptBank* getScripts() const { return &_scripts; }
  const Common::Array<Zone>& getZones() const { return _zones; }

};

}

#endif
