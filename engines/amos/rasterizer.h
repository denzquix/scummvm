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

#ifndef AMOS_RASTERIZER_H
#define AMOS_RASTERIZER_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Amos {

struct RasterizerMode {
	int visibleWidth = 0;
	int visibleHeight = 0;
	int overscanWidth = 0;
	int overscanHeight = 0;
	int visibleX = 0;
	int visibleY = 0;
	int framesPerSecond = 0;

	static RasterizerMode createDefaultPalHires();
	Common::Rect visibleRect() const;
};

class Rasterizer {
public:
	Rasterizer();

	void initRenderMode(const RasterizerMode &mode);

	Graphics::Surface &clutBuffer();
	const Graphics::Surface &clutBuffer() const;

	void setPalette(const byte *palette, uint16 start, uint16 count);
	void queueSpan(const Common::Rect &rect);
	void flushQueuedSpans();
	void endFrame();

	const RasterizerMode &mode() const;

private:
	void ensureBuffers();
	void blitSpan(const Common::Rect &span);

	RasterizerMode _mode;
	Graphics::Surface _clutSurface;
	Graphics::Surface _outputSurface;
	Common::Array<byte> _currentPalette;
	Common::Array<Common::Rect> _queuedSpans;
};

} // End of namespace Amos

#endif // AMOS_RASTERIZER_H
