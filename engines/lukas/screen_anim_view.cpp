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
#include "lukas/screen_anim_view.h"

namespace Lukas {

ScreenAnimView::ScreenAnimView(Common::Path frameContainerRes, uint firstFrame, uint frameCount, Common::Path paletteRes, Flags flags) : View("ScreenAnimView"),
  _frameContainerRes(frameContainerRes),
  _paletteRes(paletteRes),
  _firstFrame(firstFrame),
  _frameCount(frameCount),
  _nextFrame(0),
  _flags(flags) {

  _surf.create(g_engine->getScreen()->w, g_engine->getScreen()->h, Graphics::PixelFormat::createFormatCLUT8());

  g_engine->getResourceManager().loadPalettePatch(paletteRes, _fadeColors, flags & Flags::PlainPalette);
  updatePalette(1.0);
}

bool ScreenAnimView::msgFocus(const FocusMessage &msg) {
  switch (_phase) {
    case Phase::PreAnim:
    case Phase::Anim:
    case Phase::PostAnim: {
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

bool ScreenAnimView::msgKeypress(const KeypressMessage &msg) {
  if (_phase == Phase::Anim) {
    _phase = Phase::FadeOut;
    _counter = 0;
  }
	return true;
}

void ScreenAnimView::draw() {
	Graphics::ManagedSurface s = getSurface();
  s.blitFrom(_surf);
}

bool ScreenAnimView::tick() {
  switch (_phase) {
    case Phase::PreAnim: {
      if (++_counter >= _preAnimTicks) {
        _phase = Phase::Anim;
        _counter = 0;
      }
      break;
    }
    case Phase::Anim: {
      if (_nextFrame < _frameCount) {
        auto framePath = _frameContainerRes.append(Common::String::format("/%d", _firstFrame + _nextFrame));
        Common::ScopedPtr<Common::SeekableReadStream> resourceStream(g_engine->getResourceManager().loadResource(framePath));
        g_engine->getResourceManager().loadDeltaImageResource(resourceStream.get(), _surf);
        _nextFrame++;
        redraw();
      }
      else {
        _counter = 0;
        _phase = Phase::PostAnim;
      }
      break;
    }
    case Phase::PostAnim: {
      if (++_counter >= _postAnimTicks) {
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

void ScreenAnimView::updatePalette(float fadeFactor) {
  byte palette[Graphics::PALETTE_SIZE];
  g_system->getPaletteManager()->grabPalette(palette, 0, Graphics::PALETTE_COUNT);
  for (uint i = 0; i < _fadeColors.paletteData.size(); i++) {
    palette[_fadeColors.offset + i] = _fadeColors.paletteData[i] * fadeFactor;
  }
  g_system->getPaletteManager()->setPalette(palette, 0, Graphics::PALETTE_COUNT);
}

} // namespace Lukas
