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

#ifndef LUKAS_H
#define LUKAS_H

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
#include "graphics/surface.h"

#include "lukas/detection.h"
#include "lukas/events.h"
#include "lukas/resource.h"

namespace Lukas {

struct LukasGameDescription;

class LukasEngine : public Engine, public Events {
private:
	const LukasGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	ResourceManager _resourceManager;
	Common::Array<Graphics::Surface> _invIcons;
	Common::Array<Common::Array<DialogueLine>> _globalDialogue;
	Common::Array<Common::Array<DialogueLine>> _globalDialogue2;
	Common::Array<uint> _carriedInventory;
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
	LukasEngine(OSystem *syst, const LukasGameDescription *gameDesc);
	~LukasEngine() override;

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

	ResourceManager getResourceManager() {
		return _resourceManager;
	}

	bool loadCursor(Common::Path resPath, uint32 keyColor = 0) {
		Common::SharedPtr<Common::SeekableReadStream> resStream(_resourceManager.loadResource(resPath));
		return _resourceManager.loadCursorResource(resStream.get(), keyColor);
	}

	bool loadDeltaPalette(Common::Path resPath, uint32 keyColor = 0) {
		Common::SharedPtr<Common::SeekableReadStream> resStream(_resourceManager.loadResource(resPath));
		return _resourceManager.loadDeltaPaletteResource(resStream.get());
	}

	bool addInvIcon(Common::Path resPath) {
		Common::SharedPtr<Common::SeekableReadStream> resStream(_resourceManager.loadResource(resPath));
		_invIcons.resize(_invIcons.size() + 1);
		if (_resourceManager.loadIconResource(resStream.get(), _invIcons.back())) {
			// TODO: remove this testing action (auto add inventory items)
			_carriedInventory.push_back(_invIcons.size() - 1);
			return true;
		}
		else {
			_invIcons.pop_back();
			return false;
		}
	}

	uint getInvIconCount() {
		return _invIcons.size();
	}

	Graphics::Surface& getInvIcon(uint idx) {
		return _invIcons[idx];
	}

	bool getRoomResourcePath(int roomNumber, Common::Path &outPath);
	bool getRoomBackgroundResourcePath(int roomNumber, Common::Path &outPath);
	bool getRoomDialoguesResourcePath(int roomNumber, Common::Path &outPath);
	bool getRoomObjectsResourcePath(int roomNumber, Common::Path &outPath);
	bool getRoomTileDataResourcePath(int roomNumber, Common::Path &outPath);
	bool getRoomTileMapResourcePath(int roomNumber, Common::Path &outPath);

	bool loadRoom(int roomNumber, Room& room);

	Common::Array<uint>& getCarriedInventory() { return _carriedInventory; }
};

extern LukasEngine *g_engine;
#define SHOULD_QUIT ::Lukas::g_engine->shouldQuit()

} // End of namespace Lukas

#endif // LUKAS_H
