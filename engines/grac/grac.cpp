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

#include "grac/grac.h"
#include "grac/closeup.h"
#include "grac/detection.h"
#include "grac/data.h"
#include "grac/console.h"
#include "grac/amos/memorybank.h"
#include "grac/game.h"
#include "grac/room.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/cursorman.h"
#include "graphics/scaler.h"

namespace Grac {

GracEngine *g_engine;

GracEngine::GracEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Grac") {
	g_engine = this;
}

GracEngine::~GracEngine() {
	delete _game;
}

uint32 GracEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String GracEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error GracEngine::run() {
	// Initialize 640x512 paletted graphics mode
	initGraphics(640, 512);

	// Set the engine's debugger console
	setDebugger(new Console());

	Common::Path gamePath = ConfMan.getPath("path");

	int versionMajor = -1;

	for (auto fd = _gameDescription->filesDescriptions; fd->fileName; fd++) {
		if ((fd->fileType & GRACFILE_TYPEMASK) == GRACFILE_MAIN) {
			if (fd->fileType & GRACFILE_GRAC2) {
				versionMajor = 2;
			}
			else {
				versionMajor = 1;
			}
			gamePath = gamePath.append(fd->fileName);
			break;
		}
	}
	_game = new GracGame(gamePath, versionMajor);
	if (loadObjectSprites(_game->getControlsObjectBankIndex(), _controlSprites, _controlSpritesPalette)) {
		g_system->getPaletteManager()->setPalette(_controlSpritesPalette, 192);
		setNormalCursor();
		CursorMan.showMouse(true);
	}
	else {
		warning("Failed to load control sprites (GRAC %d.object)", _game->getControlsObjectBankIndex());
	}

	runGame();

	return Common::kNoError;
}

void GracEngine::setCursor(int i) {
	if (i >= 0 && (uint)i < _controlSprites.size()) {
		Graphics::Surface tmp;
		tmp.create(_controlSprites[i].surf.w * 2, _controlSprites[i].surf.h * 2, _controlSprites[i].surf.format);
		byte* pixels = (byte*)tmp.getPixels();
		Graphics::scaleBlit(
			pixels,
			(byte*)_controlSprites[i].surf.getPixels(),
			tmp.pitch,
			_controlSprites[i].surf.pitch,
			tmp.w,
			tmp.h,
			_controlSprites[i].surf.w,
			_controlSprites[i].surf.h,
			tmp.format);
		byte* endPixels = pixels + tmp.pitch * tmp.h;
		for (byte* p = pixels; p < endPixels; p++) {
			*p += 192;
		}
		CursorMan.replaceCursor(tmp, _controlSprites[i].hotspotX*2, _controlSprites[i].hotspotY*2, 192);
	}
}

void GracEngine::setNormalCursor() {
	setCursor(_controlSprites.size() - 100 + 0);
}

void GracEngine::setBusyCursor() {
	setCursor(_controlSprites.size() - 100 + 1);
}

Common::Error GracEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

static uint16 getAmiColor(const Graphics::Palette& p, uint i) {
	byte r, g, b;
	p.get(i, r, g, b);
	return ((r & 0xf0) << 4) | (g & 0xf0) | (b >> 4);
}

void GracEngine::renderSpeech(const Common::String& text, Graphics::Surface& surface, int color) {
	auto font = g_game->getSpeechFont();
	int w = font->getStringWidth(text);
	surface.create(1 + w + 1, 1 + font->getFontHeight() + 1, Graphics::PixelFormat::createFormatCLUT8());
	surface.fillRect(Common::Rect(0, 0, surface.w, surface.h), SPEECH_KEY_COLOR);
	Graphics::Palette pal = g_system->getPaletteManager()->grabPalette(0, 32);
	uint16 lightestColor, darkestColor;
	lightestColor = darkestColor = 0;
	for (uint i = 1; i < 32; i++) {
		uint16 col = getAmiColor(pal, i);
		if (col < getAmiColor(pal, darkestColor)) darkestColor = i;
		if (col > getAmiColor(pal, lightestColor)) lightestColor = i;
	}
	if (color == -1) color = lightestColor;
	font->drawString(&surface, text, 1, 0, w, darkestColor);
	font->drawString(&surface, text, 0, 1, w, darkestColor);
	font->drawString(&surface, text, 1, 2, w, darkestColor);
	font->drawString(&surface, text, 2, 1, w, darkestColor);
	font->drawString(&surface, text, 1, 1, w, color);
}

bool GracEngine::loadPicture(int pictureNumber, Graphics::Surface& outSurf, Graphics::Palette& outPalette, uint8 paletteOffset) {
	Common::FSNode fsNode;
	if (!g_game->findFile(Common::String::format("GRAC %d.picture",pictureNumber), fsNode)) {
		return false;
	}
	Common::ScopedPtr<Common::SeekableReadStream> stream(fsNode.createReadStream());
	Common::ScopedPtr<Amos::MemoryBank> abk(new Amos::MemoryBank());
	if (abk->load(stream.get())) {
		bool result = abk->toPicture(outSurf, outPalette);
		if (result) {
			if (paletteOffset) {
				byte* pixels = (byte*)outSurf.getPixels();
				byte* pixelsEnd = pixels + outSurf.pitch * outSurf.h;
				while (pixels < pixelsEnd) {
					*pixels++ += paletteOffset;
				}
			}
			return true;
		}
	}
	return false;
}

bool GracEngine::loadRoom(int roomNumber, Room& outRoom) {
	Common::FSNode fsNode;
	if (!g_game->findFile(Common::String::format("GRAC %d.room", roomNumber), fsNode)) {
		return false;
	}
	Common::SeekableReadStream* stream = decompress(fsNode.createReadStream(), false);
	if (!stream) {
		return false;
	}
	if (!outRoom.read(stream, g_game->getVersionMajor())) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool GracEngine::loadCloseup(int closeupNumber, Closeup& outCloseup) {
	Common::FSNode fsNode;
	if (!g_game->findFile(Common::String::format("GRAC %d.closeup", closeupNumber), fsNode)) {
		return false;
	}
	Common::SeekableReadStream* stream = decompress(fsNode.createReadStream(), false);
	if (!stream) {
		return false;
	}
	if (!outCloseup.read(stream)) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool GracEngine::loadSprites(const char* extension, int number, Common::Array<Amos::Sprite>& outSprites, Graphics::Palette& outPalette) {
	Common::FSNode fsNode;
	if (!g_game->findFile(Common::String::format("GRAC %d.%s", number, extension), fsNode)) {
		return false;
	}
	Common::SeekableReadStream* stream = fsNode.createReadStream();
	if (!stream) {
		return false;
	}
	if (!Amos::Sprite::loadBank(stream, outSprites, outPalette)) {
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool GracEngine::loadCharacterSprites(int number, Common::Array<Amos::Sprite>& outSprites, Graphics::Palette& outPalette) {
	return loadSprites("character", number, outSprites, outPalette);
}

bool GracEngine::loadObjectSprites(int number, Common::Array<Amos::Sprite>& outSprites, Graphics::Palette& outPalette) {
	return loadSprites("object", number, outSprites, outPalette);
}

} // End of namespace Grac
