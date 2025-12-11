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

#ifndef GRAC_DETECTION_H
#define GRAC_DETECTION_H

#include "engines/advancedDetector.h"

namespace Grac {

enum GracDebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugFilePath,
	kDebugScript,
};

enum GracFileTypes {
	GRACFILE_MAIN = 1,
	GRACFILE_INV,
	GRACFILE_CONT,
	GRACFILE_ROOM,
	GRACFILE_CHARACTER,
	GRACFILE_OBJECT,
	GRACFILE_ANIM,
	GRACFILE_CLOSEUP,
	GRACFILE_SAMPLE,
	
	GRACFILE_GRAC2 = 1 << 6,
	GRACFILE_TYPEMASK = (1 << 6)-1,
};

extern const PlainGameDescriptor gracGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Grac

class GracMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	GracMetaEngineDetection();
	~GracMetaEngineDetection() override {}

	const char *getName() const override {
		return "grac";
	}

	const char *getEngineName() const override {
		return "Grac";
	}

	const char *getOriginalCopyright() const override {
		return "Grac (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // GRAC_DETECTION_H
