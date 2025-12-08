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
#include "common/stream.h"
#include "graphics/paletteman.h"
#include "lukas/lukas.h"
#include "lukas/room_view.h"

namespace Lukas {

RoomView::RoomView(Common::Path panelImageRes, Common::Path panelPaletteRes, Flags flags) : View("RoomView"),
  _panelImageRes(panelImageRes),
  _panelPaletteRes(panelPaletteRes),
  _flags(flags) {

  if (!_panelImageRes.empty()) {
    Common::ScopedPtr<Common::SeekableReadStream> imgStream(g_engine->getResourceManager().loadResource(_panelImageRes));
    if (_flags & Flags::IconPanelImage) {
      if (!g_engine->getResourceManager().loadIconResource(imgStream.get(), _panelSurf)) {
        warning("unable to load image resource");
      }
    }
    else {
      if (!g_engine->getResourceManager().loadPlainImageResource(imgStream.get(), _panelSurf)) {
        warning("unable to load image resource");
      }
    }
    _panelPt.y = g_engine->getScreen()->getBounds().height() - _panelSurf.h;
  }

}

bool RoomView::msgFocus(const FocusMessage &msg) {
  if (!_panelPaletteRes.empty()) {
    Common::ScopedPtr<Common::SeekableReadStream> palStream(g_engine->getResourceManager().loadResource(_panelPaletteRes));
    if (!g_engine->getResourceManager().loadDeltaPaletteResource(palStream.get())) {
      warning("unable to load palette resource");
    }
  }
  return View::msgFocus(msg);
}

bool RoomView::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void RoomView::draw() {
	Graphics::ManagedSurface s = getSurface();
  s.blitFrom(_panelSurf, _panelPt);
  View::draw();
}

bool RoomView::tick() {
	return true;
}

} // namespace Lukas
