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

#ifndef LUKAS_RESOURCE_H
#define LUKAS_RESOURCE_H

#include "lukas/dialogue.h"
#include "lukas/room.h"

#include "common/array.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "graphics/surface.h"

namespace Lukas {

#define MAX_RESOURCE_SIZE (1024*1024*16)

/**
 * Resource manager for Lukas engine DAT files.
 */
class ResourceManager {
public:
	ResourceManager();
	explicit ResourceManager(const Common::Path &gameDataPath);

	/**
	 * Loads a resource by name and optional subresource indices.
	 * @param resourceName Name of the DAT file; the .DAT extension is optional.
	 * @return Stream for the requested data, or nullptr on failure.
	 */
	Common::SeekableReadStream *loadResourceFile(const Common::String &resourceName) const;

	Common::SeekableReadStream *loadResource(const Common::Path &p) const;

	struct Metadata {
		enum Type {
			Unknown = 0,
			ResourceArray = 1,
		};

		int type = 0;
		int arrayLength = 0;
		bool populated = false;
	};

	Common::HashMap<Common::Path, Metadata, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> metadata;

	Common::Array<Common::Pair<uint32, uint32>> getSubresourceSpans(Common::SeekableReadStream *resourceStream) const;

	Common::SeekableReadStream *getSubresource(Common::SeekableReadStream *resourceStream, uint32 startOffset, uint32 endOffset, DisposeAfterUse::Flag flag) const;

	Common::Array<RoomObjectInfo> getRoomObjectInfo(Common::SeekableReadStream *resourceStream, Common::CodePage page = Common::CodePage::kDos850) const;

	Common::Array<DialogueLine> getDialogue(Common::SeekableReadStream *resourceStream, Common::CodePage page = Common::CodePage::kDos850) const;

	bool loadPlainPaletteResource(Common::SeekableReadStream *resourceStream) const;

	bool loadDeltaPaletteResource(Common::SeekableReadStream *resourceStream) const;

	bool loadPlainImageResource(Common::SeekableReadStream *resourceStream, Graphics::Surface &surface) const;

	bool loadIconResource(Common::SeekableReadStream *resourceStream, Graphics::Surface &surface) const;

	bool loadCursorResource(Common::SeekableReadStream *resourceStream, uint32 keyColor = 0) const;

private:
	Common::FSNode _resourceRoot;

	Common::String normalizeName(const Common::String &resourceName) const;
	bool ensureResourceRoot(const Common::Path &gameDataPath);
	bool decompressRibInPlace(byte *buffer, uint32 compressedSize, uint32 bufferSize) const;
	Common::SeekableReadStream* toResourceStream(Common::SeekableReadStream* stream) const;
};

} // End of namespace Lukas

#endif // LUKAS_RESOURCE_H
