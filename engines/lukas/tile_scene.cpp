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
#include "common/ptr.h"
#include "graphics/paletteman.h"

#include "lukas/lukas.h"
#include "lukas/tile_scene.h"

namespace Lukas {

TileScene::TileScene(const Common::String &name, UIElement *uiParent, int roomNumber) :
  UIElement(name, uiParent),
  _roomNumber(roomNumber) {

  Common::Path path;
  if (g_engine->getRoomTileMapResourcePath(roomNumber, path)) {
    warning("tilemap path: %s", path.toString().c_str());
    Common::ScopedPtr<Common::SeekableReadStream> imgStream(g_engine->getResourceManager().loadResource(path));
    if (g_engine->getResourceManager().loadTileMapResource(imgStream.get(), _tileMap)) {
    }
    else {
      warning("Failed to load tilemap");
    }
  }
  else {
    warning("Failed to load tilemap: resource path not specified");
  }
  if (g_engine->getRoomTileDataResourcePath(roomNumber, path)) {
    warning("tilemap path: %s", path.toString().c_str());
    Common::ScopedPtr<Common::SeekableReadStream> imgStream(g_engine->getResourceManager().loadResource(path));
    if (g_engine->getResourceManager().loadTilesResource(imgStream.get(), _pal, _tiles)) {
      g_system->getPaletteManager()->setPalette(_pal.paletteData.data(), _pal.offset, _pal.paletteData.size()/3);
    }
    else {
      warning("Failed to load tile data");
    }
  }
  else {
    warning("Failed to load tile data: resource path not specified");
  }
}
TileScene::TileScene(const Common::String &name, int roomNumber) :
  UIElement(name),
  _roomNumber(roomNumber) {

  Common::Path path;
  if (g_engine->getRoomTileMapResourcePath(roomNumber, path)) {
    Common::ScopedPtr<Common::SeekableReadStream> imgStream(g_engine->getResourceManager().loadResource(path));
    g_engine->getResourceManager().loadTileMapResource(imgStream.get(), _tileMap);
  }
  if (g_engine->getRoomTileDataResourcePath(roomNumber, path)) {
    Common::ScopedPtr<Common::SeekableReadStream> imgStream(g_engine->getResourceManager().loadResource(path));
    g_engine->getResourceManager().loadTilesResource(imgStream.get(), _pal, _tiles);
    g_system->getPaletteManager()->setPalette(_pal.paletteData.data(), _pal.offset, _pal.paletteData.size()/3);
  }
}

bool TileScene::msgFocus(const FocusMessage &msg) {
  g_system->getPaletteManager()->setPalette(_pal.paletteData.data(), _pal.offset, _pal.paletteData.size()/3);
	return true;
}

bool TileScene::msgKeypress(const KeypressMessage &msg) {
	return true;
}

void TileScene::draw() {
	Graphics::ManagedSurface s = getSurface();
  for (uint ty = 0; ty < _tileMap.height; ty++) {
    for (uint tx = 0; tx < _tileMap.width; tx++) {
      byte tileIndex = _tileMap.data[ty * _tileMap.width + tx];
      s.blitFrom(_tiles, Common::Rect(0, 16*tileIndex, 16, 16*(tileIndex + 1)), Common::Point(tx * 16, ty * 16));
    }
  }
}

bool TileScene::tick() {
	return true;
}
  
} // namespace Lukas
