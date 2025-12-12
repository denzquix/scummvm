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

#include "common/system.h"
#include "common/fs.h"
#include "common/ptr.h"
#include "graphics/paletteman.h"
#include "grac/grac.h"
#include "grac/view1.h"
#include "grac/amos/memorybank.h"

namespace Grac {

bool View1::msgFocus(const FocusMessage &msg) {
	Common::FSNode picNode;
	if (g_game->findFile(Common::String::format("GRAC %d.picture", g_game->getRooms()[g_game->getStartRoom()].pictureIndex), picNode)) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(picNode.createReadStream());
		Common::ScopedPtr<Amos::MemoryBank> abk(new Amos::MemoryBank());
		if (abk->load(stream.get())) {
			warning("abk is %d bytes", abk->getBankLength());
			if (abk->toPicture(_surf, _pal)) {
				warning("Success! %dx%d (%d colors)", _surf.w, _surf.h, _pal.size());
				g_system->getPaletteManager()->setPalette(_pal);
			}
			else {
				warning("Failure...");
			}
		}
		else {
			warning("abk failed to load");
		}
	}

	return true;
}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void View1::draw() {
	Graphics::ManagedSurface s = getSurface();

	s.blitFrom(_surf, Common::Rect(0, 0, _surf.w, _surf.h), Common::Rect(0, 0, _surf.w*2, _surf.h*2), &_pal);
}

bool View1::tick() {
	return true;
}

} // namespace Grac
