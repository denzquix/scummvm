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

#ifndef LUKAS_TILE_SCENE_H
#define LUKAS_TILE_SCENE_H

#include "lukas/events.h"
#include "lukas/resource.h"

namespace Lukas {

class TileScene : public UIElement {
private:
  int _roomNumber;
	Graphics::Surface _tiles;
	Graphics::Surface _surf;
	PalettePatch _pal;
	TileMap _tileMap;

public:
	TileScene(const Common::String &name, UIElement *uiParent, int roomNumber);
	TileScene(const Common::String &name, int roomNumber);
	virtual ~TileScene() {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Lukas

#endif
