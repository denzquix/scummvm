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
#include "graphics/palette.h"
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

  g_engine->getResourceManager().loadPalettePatch(paletteRes, _fadeColors, flags & Flags::PlainPalette);
  updatePalette(0.0);
}

void SpecialScreenView::updatePalette(float fadeFactor) {
  byte palette[Graphics::PALETTE_SIZE];
  g_system->getPaletteManager()->grabPalette(palette, 0, Graphics::PALETTE_COUNT);
  for (uint i = 0; i < _fadeColors.paletteData.size(); i++) {
    palette[_fadeColors.offset + i] = _fadeColors.paletteData[i] * fadeFactor;
  }
  g_system->getPaletteManager()->setPalette(palette, 0, Graphics::PALETTE_COUNT);
}

bool SpecialScreenView::msgFocus(const FocusMessage &msg) {
  switch (_phase) {
    case Phase::PreFadeIn: {
      updatePalette(0.0);
      break;
    }
    case Phase::FadeIn: {
      updatePalette(_counter / (float)_fadeInTicks);
      break;
    }
    case Phase::Normal: {
      updatePalette(1.0);
      break;
    }
    case Phase::FadeOut: {
      updatePalette(1.0 - (_counter / (float)_fadeOutTicks));
      break;
    }
  }
	return true;
}

bool SpecialScreenView::msgKeypress(const KeypressMessage &msg) {
  if (_phase == Phase::Normal) {
    _phase = Phase::FadeOut;
    _counter = 0;
  }
	return true;
}

void SpecialScreenView::draw() {
	Graphics::ManagedSurface s = getSurface();
  s.blitFrom(_surf);
}

bool SpecialScreenView::tick() {
  switch (_phase) {
    case Phase::PreFadeIn: {
      if (++_counter >= _preFadeInTicks) {
        _phase = Phase::FadeIn;
        _counter = 0;
      }
      break;
    }
    case Phase::FadeIn: {
      updatePalette(_counter / (float)_fadeInTicks);
      if (++_counter >= _fadeInTicks) {
        updatePalette(1.0);
        _phase = Phase::Normal;
        _counter = 0;
      }
      break;
    }
    case Phase::Normal: {
      if (++_counter >= _normalTicks) {
        _phase = Phase::FadeOut;
        _counter = 0;
      }
      break;
    }
    case Phase::FadeOut: {
      updatePalette(1.0 - (_counter / (float)_fadeOutTicks));
      if (++_counter >= _fadeOutTicks) {
        updatePalette(0.0);
        _phase = Phase::Closed;
        _counter = 0;
        close();
      }
      break;
    }
  }
	return true;
}

} // namespace Lukas
