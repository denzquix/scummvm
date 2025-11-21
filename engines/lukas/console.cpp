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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/path.h"

namespace Lukas {

Console::Console() : GUI::Debugger() {
	registerCmd("help",     WRAP_METHOD(Console, Cmd_help));
	registerCmd("resfiles", WRAP_METHOD(Console, Cmd_resfiles));
}

Console::~Console() {
}

bool Console::Cmd_help(int argc, const char **argv) {
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf("  resfiles - List resource files\n");
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

} // End of namespace Lukas
