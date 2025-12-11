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
	{ "dodgygame", "The Dodgy Game" },
	{ "gractutorial", "GR.A.C. Tutorial Game" },
	{ "gracuserdisk", "GR.A.C. User Disk Game" },
	{ "twom", "The World of Magic" },
	{ "twom2", "The World of Magic II: Ghelae and the Death-Sword" },
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
	{ "samnmaxxmas", "Sam & Max" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"morristhemaggot",
		nullptr,
		{
      { "morristhemaggot.grac2", GRACFILE_GRAC2 | GRACFILE_MAIN, "eae0cd8e62fa33c40082cab1a7623e50", 3148 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lethalformula",
		"GR.A.C. 1 version",
		{
      { "Lethal_formula", GRACFILE_MAIN, "690effb9d73f9e8b8c1ba090f7a06120", 5480 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lethalformula",
		"GR.A.C. 2 version",
		{
      { "Lethal_formula", GRACFILE_GRAC2 | GRACFILE_MAIN, "96c66b8c2392151a6e9cd57096f121b4", 5556 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"dodgygame",
		nullptr,
		{
			{ "Dodgy_Game", GRACFILE_MAIN, "299932946604a38252aa0786f3498ea4", 2416 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"gractutorial",
		nullptr,
		{
			{ "tutorial.grac", GRACFILE_MAIN, "7dd7ac6672ea889ba75ff1a854cff1de", 320 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"gracuserdisk",
		nullptr,
		{
			{ "tutorial.grac", GRACFILE_MAIN, "722f4b769f27466831b5acaffd19dcc7", 640 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"twom",
		nullptr,
		{
      { "The_World_Of_Magic", GRACFILE_GRAC2 | GRACFILE_MAIN, "0fedbd44ea5f1967ba503de1be5512a8", 14636 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"twom",
		"Demo",
		{
      { "The_World_Of_Magic.grac2", GRACFILE_GRAC2 | GRACFILE_MAIN, "7f6c22e988de80bb2a41fd40f784b98b", 12860 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"twom2",
		nullptr,
		{
      { "TWOM2.GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "da4f0ca53c5ab3e29ac4322f90601c13", 10376 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"epsilon9",
		nullptr,
		{
      { "yeno.grac2", GRACFILE_GRAC2 | GRACFILE_MAIN, "379c8d9216dea09edfc7fa883615cc4d", 7572 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"entity",
		nullptr,
		{
      { "Game.GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "551afeab57a40815a6bba48203fbb194", 5236 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"experiment",
		nullptr,
		{
      { "Experiment.GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "10874b1c116de02ff3ed23f04811603a", 8024 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"experiment",
		"Demo",
		{
      { "Experiment_Demo", GRACFILE_GRAC2 | GRACFILE_MAIN, "83ec2b85c0c14dd83e2a538ffbc221bc", 8032 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest1",
		nullptr,
		{
      { "GQ1", GRACFILE_GRAC2 | GRACFILE_MAIN, "b8f5aa2f9f6468667a65df339fe7da52", 1816 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest2",
		nullptr,
		{
      { "GQ2.GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "a81a1cbe9f457dfccf71ad6fd859fcd4", 5688 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest3",
		nullptr,
		{
      { "GQ3.GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "bbff4569cd9c54ea1c04ecc448124219", 14944 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"geoquest4",
		nullptr,
		{
      { "GQ4.GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "e5dbf993a8ac31acee8bdebcdf1769cb", 7424 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"alcandria2",
		nullptr,
		{
      { "Krilok(t).GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "f90517c2e99c8c5b1713b6d5a10c29f7", 6500 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"alcandria3",
		nullptr,
		{
      { "Krilok(x).GRAC2", GRACFILE_GRAC2 | GRACFILE_MAIN, "a3e6a8fefdb35af7965969fb7e2e3c47", 11820 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"alcandria3",
		nullptr,
		{
      { "krilok(x).grac2", GRACFILE_GRAC2 | GRACFILE_MAIN, "53be90f5ff7fd56cf429d5fa8f674135", 12140 },
      AD_LISTEND
    },
		Common::ES_ESP,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"samnmaxxmas",
		"Fan Game",
		{
      { "Sam&Max.grac2", GRACFILE_GRAC2 | GRACFILE_MAIN, "7ea708d060eee1cdad76e73869df167e", 812 },
      AD_LISTEND
    },
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Grac
