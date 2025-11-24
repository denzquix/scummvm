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

#ifndef LUKAS_ROOM_H
#define LUKAS_ROOM_H

namespace Lukas {

struct RoomObjectInfo {
  uint16 x;
  uint16 y;
  uint16 width;
  uint16 height;
  uint16 id;
  Common::U32String panelName;
  Common::U32String hoverName;
  Common::U32String openText;
  Common::U32String useText;
  Common::U32String lookText;
  Common::U32String closeText;
  Common::U32String takeText;
};

} // namespace Lukas

#endif
