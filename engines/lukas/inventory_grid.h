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

#ifndef LUKAS_INVENTORY_GRID_H
#define LUKAS_INVENTORY_GRID_H

#include "lukas/events.h"
#include "lukas/resource.h"

namespace Lukas {

class InventoryGrid : public UIElement {
private:
  uint _x, _y, _slotWidth, _slotHeight, _columns, _rows;
  uint _offset = 0;

public:
	InventoryGrid(const Common::String &name, UIElement *uiParent, int x, int y, int slotWidth, int slotHeight, int columns, int rows);
	virtual ~InventoryGrid() {
	}

	void draw() override;
  void setOffset(uint v) { _offset = v; }
  uint getOffset() { return _offset; }
};

} // namespace Lukas

#endif
