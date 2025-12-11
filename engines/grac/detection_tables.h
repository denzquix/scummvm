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
	{ "epsilon9", "Epsilon 9: Revenge of the Stingons" },
	{ "entity", "Entity" },
	{ "experiment", "The Experiment" },
	{ "geoquest1", "Geo's Quest" },
	{ "geoquest2", "Geo's Quest II: In Search of Queen Lorraine" },
	{ "geoquest3", "Geo's Quest III: Gateway to Nowhere" },
	{ "geoquest4", "Geo's Quest III Part 2: There Be Cows" },
	// Note: There is no Chapter I
	{ "alcandria2", "Lord of Alcandria: Chapter II"},
	{ "alcandria3", "Lord of Alcandria: Chapter III"},
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
	{
		"entity",
		nullptr,
		AD_ENTRY1s("Game.GRAC2", "551afeab57a40815a6bba48203fbb194", 5236),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"experiment",
		nullptr,
		AD_ENTRY1s("Experiment.GRAC2", "10874b1c116de02ff3ed23f04811603a", 8024),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest1",
		nullptr,
		AD_ENTRY1s("GQ1", "b8f5aa2f9f6468667a65df339fe7da52", 1816),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest2",
		nullptr,
		AD_ENTRY1s("GQ2.GRAC2", "a81a1cbe9f457dfccf71ad6fd859fcd4", 5688),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest3",
		nullptr,
		AD_ENTRY1s("GQ3.GRAC2", "bbff4569cd9c54ea1c04ecc448124219", 14944),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest4",
		nullptr,
		AD_ENTRY1s("GQ4.GRAC2", "e5dbf993a8ac31acee8bdebcdf1769cb", 7424),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"alcandria2",
		nullptr,
		AD_ENTRY1s("Krilok(t).GRAC2", "f90517c2e99c8c5b1713b6d5a10c29f7", 6500),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"alcandria3",
		nullptr,
		AD_ENTRY1s("Krilok(x).GRAC2", "a3e6a8fefdb35af7965969fb7e2e3c47", 11820),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"alcandria3",
		nullptr,
		AD_ENTRY1s("krilok(x).grac2", "53be90f5ff7fd56cf429d5fa8f674135", 12140),
		Common::ES_ESP,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Grac
