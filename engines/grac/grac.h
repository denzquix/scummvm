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

#ifndef GRAC_H
#define GRAC_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "grac/closeup.h"
#include "grac/detection.h"
#include "grac/events.h"
#include "grac/game.h"
#include "grac/room.h"
#include "grac/amos/memorybank.h"

namespace Grac {

struct GracGameDescription;

class GracEngine : public Engine, public Events {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	GracGame *_game;
	bool loadSprites(const char* extension, int number, Common::Array<Amos::Sprite>& outSprites, Graphics::Palette& outPalette);
	Common::Array<Amos::Sprite> _controlSprites;
	Graphics::Palette _controlSpritesPalette;
protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

public:
	static const int SPEECH_KEY_COLOR = 32;

	GracEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~GracEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	void renderSpeech(const Common::String& text, Graphics::Surface& surface, int color);

	bool loadPicture(int number, Graphics::Surface& outSurf, Graphics::Palette& outPalette, uint8 paletteOffset = 0);
	bool loadRoom(int number, Room& outRoom);
	bool loadCloseup(int number, Closeup& outCloseup);
	bool loadObjectSprites(int number, Common::Array<Amos::Sprite>& outSprites, Graphics::Palette& outPalette);
	bool loadCharacterSprites(int number, Common::Array<Amos::Sprite>& outSprites, Graphics::Palette& outPalette);
	void setNormalCursor();
	void setBusyCursor();
	void setCursor(int controlSpriteIndex);
};

extern GracEngine *g_engine;
#define SHOULD_QUIT ::Grac::g_engine->shouldQuit()

} // End of namespace Grac

#endif // GRAC_H
