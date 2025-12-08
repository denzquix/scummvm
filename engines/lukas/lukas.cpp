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

	if (gameDesc->globalDialogueRes) {
		Common::ScopedPtr<Common::SeekableReadStream> ptr(_resourceManager.loadResource(gameDesc->globalDialogueRes));
		auto spans = _resourceManager.getSubresourceSpans(ptr.get());
		_globalDialogue.resize(spans.size());
		for (uint i = 0; i < spans.size(); i++) {
			Common::ScopedPtr<Common::SeekableReadStream> ptr2(_resourceManager.getSubresource(ptr.get(), spans[i].first, spans[i].second, DisposeAfterUse::NO));
			_globalDialogue[i] = _resourceManager.getDialogue(ptr2.get());
		}
	}
	else {
		_globalDialogue.clear();
	}

	if (gameDesc->globalDialogue2Res) {
		Common::ScopedPtr<Common::SeekableReadStream> ptr(_resourceManager.loadResource(gameDesc->globalDialogue2Res));
		auto spans = _resourceManager.getSubresourceSpans(ptr.get());
		_globalDialogue2.resize(spans.size());
		for (uint i = 0; i < spans.size(); i++) {
			Common::ScopedPtr<Common::SeekableReadStream> ptr2(_resourceManager.getSubresource(ptr.get(), spans[i].first, spans[i].second, DisposeAfterUse::NO));
			_globalDialogue2[i] = _resourceManager.getDialogue(ptr2.get());
		}
	}
	else {
		_globalDialogue2.clear();
	}

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
bool LukasEngine::getRoomDialoguesResourcePath(int roomNumber, Common::Path &outPath) {
	if (!_gameDescription->roomDialogueResFmt) {
		return false;
	}
	else {
		outPath = Common::String::format(_gameDescription->roomDialogueResFmt, roomNumber);
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

bool LukasEngine::loadRoom(int roomNumber, Room& room) {
	Common::Path path;
	if (getRoomObjectsResourcePath(roomNumber, path)) {
		Common::ScopedPtr<Common::SeekableReadStream> ptr(_resourceManager.loadResource(path));
		room.objects = _resourceManager.getRoomObjectInfo(ptr.get());
	}
	else {
		room.objects.clear();
	}
	if (getRoomDialoguesResourcePath(roomNumber, path)) {
		Common::ScopedPtr<Common::SeekableReadStream> ptr(_resourceManager.loadResource(path));
		auto spans = _resourceManager.getSubresourceSpans(ptr.get());
		room.dialogues.resize(spans.size());
		for (uint i = 0; i < spans.size(); i++) {
			Common::ScopedPtr<Common::SeekableReadStream> ptr2(_resourceManager.getSubresource(ptr.get(), spans[i].first, spans[i].second, DisposeAfterUse::NO));
			room.dialogues[i] = _resourceManager.getDialogue(ptr2.get());
		}
	}
	else {
		room.dialogues.clear();
	}
	return true;
}

bool LukasEngine::getRoomTileDataResourcePath(int roomNumber, Common::Path &outPath) {
	if (Common::String(_gameDescription->desc.gameId).equals("bstage")) {
		outPath = Common::String::format("LEVL%02d.DAT/3", roomNumber);
		return true;
	}
	else {
		return false;
	}
}

bool LukasEngine::getRoomTileMapResourcePath(int roomNumber, Common::Path &outPath) {
	if (Common::String(_gameDescription->desc.gameId).equals("bstage")) {
		outPath = Common::String::format("LEVL%02d.DAT/4", roomNumber);
		return true;
	}
	else {
		return false;
	}
}

} // End of namespace Lukas
