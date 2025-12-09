
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

#include "common/system.h"
#include "common/ptr.h"
#include "graphics/paletteman.h"

#include "lukas/lukas.h"
#include "lukas/inventory_grid.h"

namespace Lukas {

InventoryGrid::InventoryGrid(const Common::String &name, UIElement *uiParent, int x, int y, int slotWidth, int slotHeight, int columns, int rows) :
  UIElement(name, uiParent),
  _x(x),
  _y(y),
  _slotWidth(slotWidth),
  _slotHeight(slotHeight),
  _columns(columns),
  _rows(rows) {

}

void InventoryGrid::draw() {
	Graphics::ManagedSurface s = getSurface();
  for (uint row = 0; row < _rows; row++) {
    for (uint col = 0; col < _columns; col++) {
      uint idx = _offset + row * _columns + col;
      if (idx >= g_engine->getCarriedInventory().size()) {
        break;
      }
      uint item = g_engine->getCarriedInventory()[idx];
      auto surf = g_engine->getInvIcon(item);
      s.blitFrom(surf, Common::Point(_x + col * _slotWidth, _y + row * _slotHeight));
    }
  }
}

}
