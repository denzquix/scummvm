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

#include "amos/rasterizer.h"

#include "common/algorithm.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"

namespace Amos {

namespace {
constexpr int kPalVisibleWidth = 640;
constexpr int kPalVisibleHeight = 512;
constexpr int kPalOverscanWidth = 720;
constexpr int kPalOverscanHeight = 576;
constexpr int kPalFramesPerSecond = 50;
} // End of anonymous namespace

RasterizerMode RasterizerMode::createDefaultPalHires() {
	RasterizerMode mode;
	mode.visibleWidth = kPalVisibleWidth;
	mode.visibleHeight = kPalVisibleHeight;
	mode.overscanWidth = kPalOverscanWidth;
	mode.overscanHeight = kPalOverscanHeight;
	mode.visibleX = (mode.overscanWidth - mode.visibleWidth) / 2;
	mode.visibleY = (mode.overscanHeight - mode.visibleHeight) / 2;
	mode.framesPerSecond = kPalFramesPerSecond;
	return mode;
}

Common::Rect RasterizerMode::visibleRect() const {
	return Common::Rect(visibleX, visibleY, visibleX + visibleWidth, visibleY + visibleHeight);
}

Rasterizer::Rasterizer() {
	_currentPalette.resize(Graphics::PALETTE_SIZE);
	Common::fill(_currentPalette.begin(), _currentPalette.end(), 0);
}

void Rasterizer::initRenderMode(const RasterizerMode &mode) {
	_mode = mode;

	initGraphics(_mode.visibleWidth, _mode.visibleHeight, nullptr);
	ensureBuffers();
}

Graphics::Surface &Rasterizer::clutBuffer() {
	return _clutSurface;
}

const Graphics::Surface &Rasterizer::clutBuffer() const {
	return _clutSurface;
}

const RasterizerMode &Rasterizer::mode() const {
	return _mode;
}

void Rasterizer::setPalette(const byte *palette, uint16 start, uint16 count) {
	assert(palette);
	assert(start + count <= Graphics::PALETTE_COUNT);

	flushQueuedSpans();

	byte *dest = _currentPalette.data() + start * 3;
	Common::copy(palette, palette + count * 3, dest);
}

void Rasterizer::queueSpan(const Common::Rect &rect) {
  if (!_queuedSpans.empty()) {
		Common::Rect &lastSpan = _queuedSpans.back();
		if (lastSpan.bottom == rect.top &&
				lastSpan.left == rect.left &&
				lastSpan.right == rect.right) {
			lastSpan.bottom = rect.bottom;
		}
    return;
  }
  _queuedSpans.push_back(rect);
}

void Rasterizer::flushQueuedSpans() {
	if (_queuedSpans.empty())
		return;
	for (const auto &span : _queuedSpans)
		blitSpan(span);

	_queuedSpans.clear();
}

void Rasterizer::ensureBuffers() {
	if (_clutSurface.w != _mode.overscanWidth || _clutSurface.h != _mode.overscanHeight) {
		_clutSurface.free();
		_clutSurface.create(_mode.overscanWidth, _mode.overscanHeight, Graphics::PixelFormat::createFormatCLUT8());
	}

	if (_outputSurface.w != _mode.visibleWidth || _outputSurface.h != _mode.visibleHeight ||
			_outputSurface.format != g_system->getScreenFormat()) {
		_outputSurface.free();
		_outputSurface.create(_mode.visibleWidth, _mode.visibleHeight, g_system->getScreenFormat());
	}
}

void Rasterizer::blitSpan(const Common::Rect &span) {
	Common::Rect clip = span;
	clip.clip(_mode.visibleRect());
	if (clip.isEmpty())
		return;

	const int outX = clip.left - _mode.visibleX;
	const int outY = clip.top - _mode.visibleY;
	const Graphics::PixelFormat &format = _outputSurface.format;

	for (int y = 0; y < clip.height(); ++y) {
		const int srcY = clip.top + y;
		const int dstY = outY + y;
		const byte *src = static_cast<const byte *>(_clutSurface.getBasePtr(clip.left, srcY));

		for (int x = 0; x < clip.width(); ++x) {
			const byte index = src[x];
			const byte *palEntry = &_currentPalette[index * 3];
			_outputSurface.setPixel(outX + x, dstY, format.RGBToColor(palEntry[0], palEntry[1], palEntry[2]));
		}
	}

	g_system->copyRectToScreen(_outputSurface.getBasePtr(outX, outY), _outputSurface.pitch, outX, outY, clip.width(), clip.height());
}

void Rasterizer::endFrame() {
	flushQueuedSpans();
	g_system->updateScreen();
}

} // End of namespace Amos
