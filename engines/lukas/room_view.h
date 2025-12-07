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

#ifndef LUKAS_ROOM_VIEW_H
#define LUKAS_ROOM_VIEW_H

#include "common/path.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "lukas/view.h"

namespace Lukas {

class RoomView : public View {

public:
	enum Flags {
		None = 0,
		IconPanelImage = 1,
	};
	
private:
	Common::Path _panelImageRes;
	Common::Path _panelPaletteRes;
	Graphics::Surface _panelSurf;
  Common::Point _panelPt;
	Flags _flags;

public:
	RoomView(Common::Path panelImageRes, Common::Path panelPaletteRes, Flags flags = Flags::None);
	virtual ~RoomView() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;

};

} // namespace Lukas

#endif
