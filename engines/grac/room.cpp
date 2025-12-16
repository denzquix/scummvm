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

#include "grac/room.h"
#include "grac/data.h"

namespace Grac {

bool Room::read(Common::SeekableReadStream* fromStream, int versionMajor) {

  if (!fromStream) {
    return false;
  }

  const uint walkZoneCount = 32;
  const uint backgroundObjectCount = versionMajor >= 2 ? 32 : 16;
  const uint foregroundObjectCount = 16;
  const uint walkPointCount = 16;
  const uint stringCount = versionMajor >= 2 ? 1000 : 0;
  const uint roomScriptCount = 50;

  _walkZones.resize(walkZoneCount);
  _backgroundObjects.resize(backgroundObjectCount);
  _foregroundObjects.resize(foregroundObjectCount);
  _walkPoints.resize(walkPointCount);

  for (uint i = 0; i < 16; i++) {
    // 32 walk zones
    _walkZones[i].read(fromStream, versionMajor);
    _walkZones[i + 16].read(fromStream, versionMajor);
    // 16/32 background objects
    if (backgroundObjectCount == 16) {
      _backgroundObjects[15 - i].read(fromStream);
    }
    else {
      _backgroundObjects[i].read(fromStream);
      _backgroundObjects[i + 16].read(fromStream);
    }
    // 16 foreground objects
    _foregroundObjects[i].read(fromStream);
    // 16 walk points
    _walkPoints[i].read(fromStream);
  }

  if (fromStream->err()) {
    error("Stream read error");
  }
  if (fromStream->eos()) {
    error("Unexpected end of room data");
  }

  if (versionMajor < 2) {
    if (!fromStream->skip(100)) {
      error("error skipping unused bytes");
    }
  }

  if (!_scriptBank.read(fromStream, roomScriptCount, 101, versionMajor)) {
    error("Failed to load room script bank");
  }

  _strings.resize(stringCount);
  for (uint str_i = 0; str_i < stringCount; str_i++) {
    if (!readTerminatedString(fromStream, _strings[str_i])) {
      error("Failed to read room string");
    }
  }

  for (uint i = 0; i < 16; i++) {
    if (backgroundObjectCount == 16) {
      if (!readTerminatedString(fromStream, _backgroundObjects[15 - i].name)) {
        error("Failed to read object name");
      }
    }
    else {
      if (!readTerminatedString(fromStream, _backgroundObjects[i].name)) {
        error("Failed to read object name");
      }
      if (!readTerminatedString(fromStream, _backgroundObjects[i + 16].name)) {
        error("Failed to read object name");
      }
    }
    if (!readTerminatedString(fromStream, _foregroundObjects[i].name)) {
      error("Failed to read object name");
    }
  }

  if (fromStream->err()) {
    error("Stream read error");
  }
  if (fromStream->eos()) {
    error("Unexpected end of room data");
  }

  return true;

}

void Room::BackgroundObject::read(Common::SeekableReadStream *fromStream) {
  if (!fromStream) {
    return;
  }
  hotspot.left = fromStream->readSint16BE();
  hotspot.top = fromStream->readSint16BE();
  hotspot.right = fromStream->readSint16BE();
  hotspot.bottom = fromStream->readSint16BE();
  pastePos.x = fromStream->readSint16BE();
  pastePos.y = fromStream->readSint16BE();
  message = fromStream->readSint16BE();
  altitude = fromStream->readSint16BE();
  approach = fromStream->readSint16BE();
  walkZone = fromStream->readSint16BE();
  enabledFlag = fromStream->readSint16BE();
  const uint verbCount = 10;
  verbScripts.resize(verbCount);
  for (uint verb_i = 0; verb_i < verbCount; verb_i++) {
    verbScripts[verb_i] = fromStream->readSint16BE();
  }
  goToPoint.x = fromStream->readSint16BE();
  goToPoint.y = fromStream->readSint16BE();
}

void Room::ForegroundObject::read(Common::SeekableReadStream *fromStream) {
  if (!fromStream) {
    return;
  }
  goToPoint.x = fromStream->readSint16BE();
  goToPoint.y = fromStream->readSint16BE();
  spriteImage = fromStream->readSint16BE();
  inventoryItem = fromStream->readSint16BE();
  spritePos.x = fromStream->readSint16BE();
  spritePos.y = fromStream->readSint16BE();
  message = fromStream->readSint16BE();
  altitude = fromStream->readSint16BE();
  approach = fromStream->readSint16BE();
  walkZone = fromStream->readSint16BE();
  enabledFlag = fromStream->readSint16BE();
  const uint verbCount = 10;
  verbScripts.resize(verbCount);
  for (uint verb_i = 0; verb_i < verbCount; verb_i++) {
    verbScripts[verb_i] = fromStream->readSint16BE();
  }
}

void Room::WalkZone::read(Common::SeekableReadStream *fromStream, int versionMajor) {
  rect.left = fromStream->readSint16BE();
  rect.top = fromStream->readSint16BE();
  rect.right = fromStream->readSint16BE();
  rect.bottom = fromStream->readSint16BE();
  neighbors.resize(4);
  neighbors[0] = fromStream->readSint16BE();
  neighbors[1] = fromStream->readSint16BE();
  neighbors[2] = fromStream->readSint16BE();
  neighbors[3] = fromStream->readSint16BE();
  perspectiveScale1 = fromStream->readSint16BE();
  perspectiveScale2 = fromStream->readSint16BE();
  enabledFlag = fromStream->readSint16BE();
  enterScript = fromStream->readSint16BE();
}

void Room::WalkPoint::read(Common::SeekableReadStream *fromStream) {
  point.x = fromStream->readSint16BE();
  point.y = fromStream->readSint16BE();
  faceDir = fromStream->readSint16BE();
  walkZone = fromStream->readSint16BE();
}

}
