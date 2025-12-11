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

namespace Grac {

const PlainGameDescriptor gracGames[] = {
	{ "morristhemaggot", "Morris the Maggot" },
	{ "lethalformula", "Lethal Formula" },
	{ "twom", "The World of Magic" },
	{ "epsilon9", "Epsilon 9" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"morristhemaggot",
		nullptr,
		AD_ENTRY1s("morristhemaggot.grac2", "eae0cd8e62fa33c40082cab1a7623e50", 3148),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lethalformula",
		"GR.A.C. 2 version",
		AD_ENTRY1s("Lethal_formula", "96c66b8c2392151a6e9cd57096f121b4", 5556),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"twom",
		"Demo",
		AD_ENTRY1s("The_World_Of_Magic.grac2", "7f6c22e988de80bb2a41fd40f784b98b", 12860),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"epsilon9",
		nullptr,
		AD_ENTRY1s("yeno.grac2", "379c8d9216dea09edfc7fa883615cc4d", 7572),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Grac
