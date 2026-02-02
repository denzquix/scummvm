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

namespace Amos {

const PlainGameDescriptor amosGames[] = {
	{ "starbase13", "Starbase 13" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"starbase13",
		nullptr,
		{
			{"starbase_13", 0, "ced973d9e77f58c6ee3859d258a41231", 286104},			
			{"main", 0, "638473a2cc33b880847780e860261082", 259640},
			AD_LISTEND,
		},
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Amos
