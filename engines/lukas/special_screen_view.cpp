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
#include "lukas/special_screen_view.h"

namespace Lukas {

SpecialScreenView::SpecialScreenView(Common::Path imageRes, Common::Path paletteRes, Flags flags) : View("SpecialScreenView"),
  _imageRes(imageRes),
  _paletteRes(paletteRes),
  _flags(flags) {

  if (!_imageRes.empty()) {
    Common::ScopedPtr<Common::SeekableReadStream> imgStream(g_engine->getResourceManager().loadResource(_imageRes));
    if (!g_engine->getResourceManager().loadPlainImageResource(imgStream.get(), _surf)) {
      warning("unable to load image resource");
    }
  }

}

bool SpecialScreenView::msgFocus(const FocusMessage &msg) {
  if (!_paletteRes.empty()) {
    Common::ScopedPtr<Common::SeekableReadStream> palStream(g_engine->getResourceManager().loadResource(_paletteRes));
    if (_flags & Flags::PlainPalette) {
      if (!g_engine->getResourceManager().loadPlainPaletteResource(palStream.get())) {
        warning("unable to load palette resource");
      }
    }
    else {
      if (!g_engine->getResourceManager().loadDeltaPaletteResource(palStream.get())) {
        warning("unable to load palette resource");
      }
    }
  }
	return true;
}

bool SpecialScreenView::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void SpecialScreenView::draw() {
	Graphics::ManagedSurface s = getSurface();
  s.blitFrom(_surf);
}

bool SpecialScreenView::tick() {
	return true;
}

} // namespace Lukas
