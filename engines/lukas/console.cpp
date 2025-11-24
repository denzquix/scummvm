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

#include "lukas/console.h"
#include "lukas/lukas.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/path.h"

namespace Lukas {

Console::Console(LukasEngine *engine) : GUI::Debugger() {
	_engine = engine;
	registerCmd("help",     WRAP_METHOD(Console, Cmd_help));
	registerCmd("resfiles", WRAP_METHOD(Console, Cmd_resfiles));
	registerCmd("subres",   WRAP_METHOD(Console, Cmd_subres));
	registerCmd("roomobjs", WRAP_METHOD(Console, Cmd_roomobjs));
}

Console::~Console() {
}

bool Console::Cmd_help(int argc, const char **argv) {
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf("  resfiles - List resource files\n");
	debugPrintf("  subres <res> - List subresources of resource\n");
	debugPrintf("  roomobjs <res> - View room object info resource\n");
	debugPrintf("\n");
	return true;
}

bool Console::Cmd_resfiles(int argc, const char **argv) {
	Common::Path basePath = ConfMan.getPath("path");
	Common::FSNode baseDir = Common::FSNode(basePath);
	Common::FSNode resourceDir = baseDir.getChild("RESOURCE");
	if (!resourceDir.isDirectory()) {
		error("Resource directory not found!\n");
		return false;
	}
	Common::FSList resourceFiles;
	if (!resourceDir.getChildren(resourceFiles, Common::FSNode::ListMode::kListFilesOnly)) {
		error("Unable to iterate resource directory\n");
		return false;
	}
	debugPrintf("\n");
	debugPrintf("Resource files\n");
	debugPrintf("--------------\n");
	if (resourceFiles.empty()) {
		debugPrintf("  (none found!)");
	}
	else {
		for (auto &i : resourceFiles) {
			Common::String name = i.getFileName();
			debugPrintf("  %s\n", name.c_str());
		}
	}
	debugPrintf("\n");
	return true;
}

bool Console::Cmd_subres(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: subres <res>\n");
		return true;
	}
	auto resman = _engine->getResourceManager();
	Common::ScopedPtr<Common::SeekableReadStream> stream(resman.loadResource(Common::Path(argv[1])));
	if (!stream) {
		debugPrintf("Resource not found\n");
		return true;
	}
	debugPrintf("\n");
	debugPrintf("Subresources\n");
	debugPrintf("------------\n");
	auto subres = resman.getSubresourceSpans(stream.get());
	if (subres.empty()) {
		debugPrintf("  (none found!)\n");
	}
	else {
		for (uint i = 0; i < subres.size(); i++) {
			if (subres[i].first == subres[i].second) {
				continue;
			}
			debugPrintf("  %s/%d\n", argv[1], i);
		}
	}
	debugPrintf("\n");
	return true;
}

bool Console::Cmd_roomobjs(int argc, const char **argv) {
	const Common::CodePage page = Common::CodePage::kDos850;
	if (argc != 2) {
		debugPrintf("Usage: roomobjs <res>\n");
		return true;
	}
	auto resman = _engine->getResourceManager();
	Common::ScopedPtr<Common::SeekableReadStream> stream(resman.loadResource(Common::Path(argv[1])));
	if (!stream) {
		debugPrintf("Resource not found\n");
		return true;
	}
	debugPrintf("\n");
	debugPrintf("Room Objects\n");
	debugPrintf("------------\n");
	auto roomobjs = resman.getRoomObjectInfo(stream.get(), page);
	if (roomobjs.empty()) {
		debugPrintf("  (none found!)\n");
	}
	else {
		for (uint i = 0; i < roomobjs.size(); i++) {
			debugPrintf("[%d] X:%d Y:%d W:%d H:%d \"%s\" \"%s\"\n",
				roomobjs[i].id,
				roomobjs[i].x, roomobjs[i].y, roomobjs[i].width, roomobjs[i].height,
				Common::toPrintable(roomobjs[i].panelName.encode(page)).c_str(),
				Common::toPrintable(roomobjs[i].hoverName.encode(page)).c_str());
			if (!roomobjs[i].lookText.empty()) {
				Common::String str = Common::toPrintable(roomobjs[i].lookText.encode(page), false);
				debugPrintf("- look: \"%s\"\n", str.c_str());
			}
			if (!roomobjs[i].useText.empty()) {
				Common::String str = Common::toPrintable(roomobjs[i].useText.encode(page), false);
				debugPrintf("- use: \"%s\"\n", str.c_str());
			}
			if (!roomobjs[i].takeText.empty()) {
				Common::String str = Common::toPrintable(roomobjs[i].takeText.encode(page), false);
				debugPrintf("- take: \"%s\"\n", str.c_str());
			}
			if (!roomobjs[i].openText.empty()) {
				Common::String str = Common::toPrintable(roomobjs[i].openText.encode(page), false);
				debugPrintf("- open: \"%s\"\n", str.c_str());
			}
			if (!roomobjs[i].closeText.empty()) {
				Common::String str = Common::toPrintable(roomobjs[i].closeText.encode(page), false);
				debugPrintf("- close: \"%s\"\n", str.c_str());
			}
		}
	}
	debugPrintf("\n");
	return true;
}

} // End of namespace Lukas
