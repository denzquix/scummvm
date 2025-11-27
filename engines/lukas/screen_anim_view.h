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

#ifndef LUKAS_SCREEN_ANIM_VIEW_H
#define LUKAS_SCREEN_ANIM_VIEW_H

#include "common/path.h"
#include "graphics/surface.h"
#include "lukas/view.h"

namespace Lukas {

class ScreenAnimView : public View {

public:
	enum Flags {
		None = 0,
		PlainPalette = 1,
	};
	
private:
	Common::Path _frameContainerRes;
	Common::Path _paletteRes;
	Graphics::Surface _surf;
  uint _firstFrame;
  uint _frameCount;
  uint _nextFrame;
	Flags _flags;

public:
	ScreenAnimView(Common::Path frameContainer, uint firstFrame, uint frameCount, Common::Path paletteRes, Flags flags = Flags::None);
	virtual ~ScreenAnimView() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;

};

} // namespace Lukas

#endif
