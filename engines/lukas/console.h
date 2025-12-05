
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

#ifndef LUKAS_CONSOLE_H
#define LUKAS_CONSOLE_H

#include "lukas/lukas.h"
#include "gui/debugger.h"

namespace Lukas {

class Console : public GUI::Debugger {
private:
	bool Cmd_help(int argc, const char **argv);
	bool Cmd_resfiles(int argc, const char **argv);
	bool Cmd_subres(int argc, const char **argv);
	bool Cmd_roomobjs(int argc, const char **argv);
	bool Cmd_dlgres(int argc, const char **argv);
	bool Cmd_findres(int argc, const char **argv);
	LukasEngine *_engine;
	void debugPrintControlString(const Common::U32String &ustr, Common::CodePage page);
public:
	Console(LukasEngine *engine);
	~Console() override;
};

} // End of namespace Lukas

#endif // LUKAS_CONSOLE_H
