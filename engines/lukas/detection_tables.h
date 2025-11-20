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

namespace Lukas {

const PlainGameDescriptor lukasGames[] = {
	{ "bifi2", "Bi-Fi Roll: Action in Hollywood" },
	{ "bstage", "Backstage" },
	{ "capzins", "Captain Zins" },
	{ "schatten", "Dunkle Schatten" },
	{ "telekom2", "Das Telekommando Kehrt Zurück" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"bifi2",
		nullptr,
		{
			{"xp00.exe", 0, "f5e23587bea3a83b825ce0bff82e9fd8", 33031},
			AD_LISTEND,
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"bstage",
		nullptr,
		{
			{"xp00.exe", 0, "861dfd29731049b0b9ea87fa0861db68", 57447},
			AD_LISTEND,
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"capzins",
		nullptr,
		{
  		{"xp00.exe", 0, "863fb3bdbecc62730c2fa1f617b57b30", 38007},
			AD_LISTEND,
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"schatten",
		nullptr,
		{
  		{"xp00.exe", 0, "61a4147d354628f7e3684407b5ca17b8", 33367},
			AD_LISTEND,
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"telekom2",
		nullptr,
		{
  		{"xp00.exe", 0, "5a05d28efd68596852cc560481833168", 31943},
			AD_LISTEND,
		},
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Lukas
