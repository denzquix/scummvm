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

namespace Loomes {

const PlainGameDescriptor loomesGames[] = {
	{ "vl", "Victor Loomes" },
	{ "enviro", "Die Enviro-Kids Greifen Ein" },
	{ "jeffjet", "Jeff Jet: Abenteuer Infohighway" },
	{ "hfa", "Hilfe für Amajabere" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"vl",
		nullptr,
		AD_ENTRY1s("DATA.-1-", "ea9e87f129b8b5fda8b20c18c3c48268", 1009597),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"enviro",
		nullptr,
		AD_ENTRY1s("DATA.-1-", "ddd68b75745bafa917171a86c31d9de9", 7609296),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"jeffjet",
		nullptr,
		AD_ENTRY1s("DATA.-1-", "be777ce50cb692bbfc68dcb41d482940", 1404960),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"hfa",
		nullptr,
		AD_ENTRY1s("DATA.-1-", "dfab880d6fbd374c1c80c9610f95a301", 506066),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Loomes
