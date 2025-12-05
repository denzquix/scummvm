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

#include "lukas/lukas.h"
#include "lukas/detection.h"
#include "lukas/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

namespace Lukas {

LukasEngine *g_engine;

LukasEngine::LukasEngine(OSystem *syst, const LukasGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Lukas"),
	_resourceManager(ConfMan.getPath("path")) {
	g_engine = this;
}

LukasEngine::~LukasEngine() {
}

uint32 LukasEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String LukasEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error LukasEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	// Set the engine's debugger console
	setDebugger(new Console(this));

	runGame();

	return Common::kNoError;
}

Common::Error LukasEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

bool LukasEngine::getRoomResourcePath(int roomNumber, Common::Path &outPath) {
	if (!_gameDescription->roomRootResFmt) {
		return false;
	}
	else {
		outPath = Common::String::format(_gameDescription->roomRootResFmt, roomNumber);
		return true;
	}
}
bool LukasEngine::getRoomBackgroundResourcePath(int roomNumber, Common::Path &outPath) {
	if (!_gameDescription->roomBgResFmt) {
		return false;
	}
	else {
		outPath = Common::String::format(_gameDescription->roomBgResFmt, roomNumber);
		return true;
	}
}
bool LukasEngine::getRoomDialogueResourcePath(int roomNumber, int dialogueNumber, Common::Path &outPath) {
	if (!_gameDescription->roomDialogueResFmt) {
		return false;
	}
	else {
		outPath = Common::String::format(_gameDescription->roomDialogueResFmt, roomNumber, dialogueNumber);
		return true;
	}
}
bool LukasEngine::getRoomObjectsResourcePath(int roomNumber, Common::Path &outPath) {
	if (!_gameDescription->roomObjectsResFmt) {
		return false;
	}
	else {
		outPath = Common::String::format(_gameDescription->roomObjectsResFmt, roomNumber);
		return true;
	}
}

} // End of namespace Lukas
