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

#include "common/stream.h"
#include "grac/closeup.h"

namespace Grac {

bool Closeup::read(Common::SeekableReadStream *fromStream) {
  if (!fromStream) {
    return false;
  }

  _zones.resize(kZoneCount);
  for (uint i = 0; i < kZoneCount; i++) {
    _zones[i].read(fromStream);
  }

  if (!_scripts.read(fromStream, 25, 101, 2)) {
    warning("Failed to read closeup scripts");
    return false;
  }

  return true;
}

void Closeup::Zone::read(Common::SeekableReadStream *fromStream) {
  if (!fromStream) {
    return;
  }
  downImage = fromStream->readSint16BE();
  hotspot.left = fromStream->readSint16BE();
  hotspot.top = fromStream->readSint16BE();
  hotspot.right = fromStream->readSint16BE();
  hotspot.bottom = fromStream->readSint16BE();
  imagePosition.x = fromStream->readSint16BE();
  imagePosition.y = fromStream->readSint16BE();
  upImage = fromStream->readSint16BE();
  clickScript = fromStream->readSint16BE();
}

}
