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

#include "grac/console.h"
#include "grac/grac.h"
#include "grac/script.h"

namespace Grac {

Console::Console() : GUI::Debugger() {
	registerCmd("viewscript", WRAP_METHOD(Console, Cmd_viewscript));
}

Console::~Console() {
}

bool Console::Cmd_viewscript(int argc, const char **argv) {
	const ScriptBank* scriptBank = nullptr;
	uint64 num = 0;
	if (argc == 3) {
		Common::String arg1(argv[1]);
		Common::String arg2(argv[2]);
		num = arg2.asUint64();
		if (arg1 == "char") {
			scriptBank = g_game->getCharacterScripts();
		}
		else if (arg1 == "verb") {
			scriptBank = g_game->getVerbScripts();
		}
		else if (arg1 == "inv") {
			scriptBank = g_game->getInventoryScripts();
		}
	}
	if (scriptBank == nullptr) {
		debugPrintf("Usage: viewscript (char|verb|inv) X (X = script number)\n");
		return true;
	}
	if (num >= scriptBank->scripts.size()) {
		debugPrintf("Script number out of range\n");
		return true;
	}
	debugPrintf("Script %d\n========\n", (int)num);
	for (uint instr_i = 0; instr_i < scriptBank->scripts[num].size(); instr_i++) {
		auto instr = &scriptBank->scripts[num][instr_i];
		debugPrintf("%s\n", instr->toString().c_str());
	}
	debugPrintf("\n");
	return true;
}

} // End of namespace Grac
