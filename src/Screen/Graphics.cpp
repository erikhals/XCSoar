/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/Graphics.hpp"
#include "Screen/UnitSymbol.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Ramp.hpp"
#include "Appearance.hpp"
#include "SettingsUser.hpp"
#include "resource.h"
#include "Asset.hpp"
#include "LogFile.hpp"

#define NUMSNAILRAMP 6

const COLORRAMP snail_colors[] = {
  {0,   0xff, 0x3e, 0x00},
  {50,  0xcd, 0x4f, 0x27},
  {100, 0x8f, 0x8f, 0x8f},
  {150, 0x27, 0xcd, 0x4f},
  {201, 0x00, 0xff, 0x3e},
  {501, 0x00, 0xff, 0x3e}
};

// airspace brushes/colours
const Color
ScreenGraphics::GetAirspaceColour(const int i)
{
  return Colours[i];
}

const Brush &
ScreenGraphics::GetAirspaceBrush(const int i)
{
  return hAirspaceBrushes[i];
}

const Color
ScreenGraphics::GetAirspaceColourByClass(const int i,
    const SETTINGS_MAP &settings)
{
  return Colours[settings.iAirspaceColour[i]];
}

const Brush &
ScreenGraphics::GetAirspaceBrushByClass(const int i,
    const SETTINGS_MAP &settings)
{
  return hAirspaceBrushes[settings.iAirspaceBrush[i]];
}

const Color ScreenGraphics::ColorSelected = Color(0xC0, 0xC0, 0xC0);
const Color ScreenGraphics::ColorUnselected = Color::WHITE;
const Color ScreenGraphics::ColorWarning = Color::RED;
const Color ScreenGraphics::ColorOK = Color::BLUE;
const Color ScreenGraphics::ColorBlack = Color::BLACK;
const Color ScreenGraphics::ColorMidGrey = Color::GRAY;

const Color ScreenGraphics::inv_redColor = Color(0xff, 0x70, 0x70);
const Color ScreenGraphics::inv_blueColor = Color(0x90, 0x90, 0xff);
const Color ScreenGraphics::inv_yellowColor = Color::YELLOW;
const Color ScreenGraphics::inv_greenColor = Color::GREEN;
const Color ScreenGraphics::inv_magentaColor = Color::MAGENTA;

const Color ScreenGraphics::TaskColor = Color(0, 120, 0);
const Color ScreenGraphics::BackgroundColor = Color::WHITE;

const Color ScreenGraphics::Colours[] = {
  Color::RED,
  Color::GREEN,
  Color::BLUE,
  Color::YELLOW,
  Color::MAGENTA,
  Color::CYAN,
  Color(0x7F, 0x00, 0x00),
  Color(0x00, 0x7F, 0x00),
  Color(0x00, 0x00, 0x7F),
  Color(0x7F, 0x7F, 0x00),
  Color(0x7F, 0x00, 0x7F),
  Color(0x00, 0x7F, 0x7F),
  Color::WHITE,
  Color(0xC0, 0xC0, 0xC0),
  Color(0x7F, 0x7F, 0x7F),
  Color::BLACK,
};

void
ScreenGraphics::Initialise()
{
  /// @todo enhancement: support red/green color blind pilots with adjusted colour scheme

  int i;

  LogStartUp(_T("Initialise graphics"));

  LoadUnitSymbols();

  infoSelectedBrush.set(MapGfx.ColorSelected);
  infoUnselectedBrush.set(MapGfx.ColorUnselected);

  AlarmBrush.set(Color::RED);
  WarningBrush.set(Color(0xFF, 0xA2, 0x00));
  TrafficBrush.set(Color::GREEN);

  hBackgroundBrush.set(BackgroundColor);

  hFLARMTraffic.load(IDB_FLARMTRAFFIC);
  hTerrainWarning.load(IDB_TERRAINWARNING);
  hGPSStatus1.load(IDB_GPSSTATUS1, false);
  hGPSStatus2.load(IDB_GPSSTATUS2, false);
  hLogger.load_big(IDB_LOGGER, IDB_LOGGER_HD);
  hLoggerOff.load_big(IDB_LOGGEROFF, IDB_LOGGEROFF_HD);
  hBmpTeammatePosition.load(IDB_TEAMMATE_POS);

  hAutoMacCready.load(IDB_AUTOMACCREADY, false);
  hCruise.load(IDB_CRUISE, false);
  hClimb.load(IDB_CLIMB, false);
  hFinalGlide.load(IDB_FINALGLIDE, false);
  hAbort.load(IDB_ABORT, false);

  // airspace brushes and colors
  hAirspaceBitmap[0].load(IDB_AIRSPACE0);
  hAirspaceBitmap[1].load(IDB_AIRSPACE1);
  hAirspaceBitmap[2].load(IDB_AIRSPACE2);
  hAirspaceBitmap[3].load(IDB_AIRSPACE3);
  hAirspaceBitmap[4].load(IDB_AIRSPACE4);
  hAirspaceBitmap[5].load(IDB_AIRSPACE5);
  hAirspaceBitmap[6].load(IDB_AIRSPACE6);
  hAirspaceBitmap[7].load(IDB_AIRSPACE7);
  hAirspaceInterceptBitmap.load(IDB_AIRSPACEI);

  hAboveTerrainBitmap.load(IDB_ABOVETERRAIN);

  for (i = 0; i < NUMAIRSPACEBRUSHES; i++)
    hAirspaceBrushes[i].set(hAirspaceBitmap[i]);

  hAboveTerrainBrush.set(hAboveTerrainBitmap);

  hbWind.set(Color::GRAY);

  hBmpMapScale.load(IDB_MAPSCALE_A);
  hBrushFlyingModeAbort.set(Color::RED);

  hBmpThermalSource.load(IDB_THERMALSOURCE);
  hBmpTarget.load(IDB_TARGET);

  hbCompass.set(has_colors() ? Color(0x40, 0x40, 0xFF) : Color::WHITE);

  hbThermalBand.set(Color(0x80, 0x80, 0xFF));
  hbBestCruiseTrack.set(Color::BLUE);
  hbFinalGlideBelow.set(Color::RED);
  hbFinalGlideBelowLandable.set(Color(0xFF, 180, 0x00));
  hbFinalGlideAbove.set(Color::GREEN);

  hpCompassBorder.set(Layout::Scale(3), Color::WHITE);

  hpWind.set(Layout::Scale(2), Color::BLACK);

  hpWindThick.set(Layout::Scale(4), Color(255, 220, 220));

  hpBearing.set(Layout::Scale(2), Color::BLACK);
  hpBestCruiseTrack.set(Layout::Scale(1), Color::BLUE);
  hpCompass.set(Layout::Scale(1), has_colors()
                ? Color(0xcf, 0xcf, 0xFF) : Color::BLACK);
  hpThermalBand.set(Layout::Scale(2), Color(0x40, 0x40, 0xFF));
  hpThermalBandGlider.set(Layout::Scale(2), Color(0x00, 0x00, 0x30));

  hpFinalGlideBelow.set(Layout::Scale(1), Color(0xFF, 0xA0, 0xA0));
  hpFinalGlideBelowLandable.set(Layout::Scale(1), Color(255, 196, 0));

  hpFinalGlideAbove.set(Layout::Scale(1), Color(0xA0, 0xFF, 0xA0));

  hpSpeedSlow.set(Layout::Scale(1), Color::RED);
  hpSpeedFast.set(Layout::Scale(1), Color::GREEN);

  hpStartFinishThick.set(Layout::Scale(5), TaskColor);

  hpStartFinishThin.set(Layout::Scale(1), Color::RED);

  hpMapScale.set(Layout::Scale(1), Color::BLACK);
  hpTerrainLine.set(Pen::DASH, Layout::Scale(1), Color(0x30, 0x30, 0x30));
  hpTerrainLineBg.set(Layout::Scale(1), Color::WHITE);

  hpVisualGlideLightBlack.set(Pen::DASH, Layout::Scale(1), Color::BLACK);
  hpVisualGlideHeavyBlack.set(Pen::DASH, Layout::Scale(2), Color::BLACK);
  hpVisualGlideLightRed.set(Pen::DASH, Layout::Scale(1), Color::RED);
  hpVisualGlideHeavyRed.set(Pen::DASH, Layout::Scale(2), Color::RED);

  SmallIcon.load(IDB_SMALL);
  TurnPointIcon.load(IDB_TURNPOINT);

  hpAircraft.set(Layout::Scale(3), Color::WHITE);
  hpAircraftBorder.set(Layout::Scale(1), Color::BLACK);
}

void
ScreenGraphics::InitialiseConfigured(const SETTINGS_MAP &settings_map)
{
  InitSnailTrail(settings_map);
  InitLandableIcons();
  InitAirspacePens(settings_map);
}

void
ScreenGraphics::InitSnailTrail(const SETTINGS_MAP &settings_map)
{
  int iwidth;
  int minwidth;
  minwidth = max(Layout::Scale(2),
                 Layout::Scale(settings_map.SnailWidthScale) / 16);

  for (int i = 0; i < NUMSNAILCOLORS; i++) {
    short ih = i * 200 / (NUMSNAILCOLORS - 1);
    hSnailColours[i] = ColorRampLookup(ih, snail_colors, NUMSNAILRAMP, 6);

    if (i < NUMSNAILCOLORS / 2)
      iwidth = minwidth;
    else
      iwidth = max(minwidth, (i - NUMSNAILCOLORS / 2)
          * Layout::Scale(settings_map.SnailWidthScale) / NUMSNAILCOLORS);

    hSnailPens[i].set(iwidth, hSnailColours[i]);
  }
}

void
ScreenGraphics::InitLandableIcons()
{
  if (Appearance.IndLandable == wpLandableDefault) {
    AirportReachableIcon.load_big(IDB_REACHABLE, IDB_REACHABLE_HD);
    AirportUnreachableIcon.load_big(IDB_LANDABLE, IDB_LANDABLE_HD);
    FieldReachableIcon.load_big(IDB_REACHABLE, IDB_REACHABLE_HD);
    FieldUnreachableIcon.load_big(IDB_LANDABLE, IDB_LANDABLE_HD);
  } else if (Appearance.IndLandable == wpLandableAltA) {
    AirportReachableIcon.load_big(IDB_AIRPORT_REACHABLE,
                                  IDB_AIRPORT_REACHABLE_HD);
    AirportUnreachableIcon.load_big(IDB_AIRPORT_UNREACHABLE,
                                    IDB_AIRPORT_UNREACHABLE_HD);
    FieldReachableIcon.load_big(IDB_OUTFIELD_REACHABLE,
                                IDB_OUTFIELD_REACHABLE_HD);
    FieldUnreachableIcon.load_big(IDB_OUTFIELD_UNREACHABLE,
                                  IDB_OUTFIELD_UNREACHABLE_HD);
  } else if (Appearance.IndLandable == wpLandableAltB) {
    AirportReachableIcon.load_big(IDB_AIRPORT_REACHABLE,
                                  IDB_AIRPORT_REACHABLE_HD);
    AirportUnreachableIcon.load_big(IDB_AIRPORT_UNREACHABLE2,
                                    IDB_AIRPORT_UNREACHABLE2_HD);
    FieldReachableIcon.load_big(IDB_OUTFIELD_REACHABLE,
                                IDB_OUTFIELD_REACHABLE_HD);
    FieldUnreachableIcon.load_big(IDB_OUTFIELD_UNREACHABLE2,
                                  IDB_OUTFIELD_UNREACHABLE2_HD);
  }
}

void
ScreenGraphics::InitAirspacePens(const SETTINGS_MAP &settings_map)
{
  for (int i = 0; i < AIRSPACECLASSCOUNT; i++)
    hAirspacePens[i].set(Layout::Scale(2),
                         GetAirspaceColourByClass(i, settings_map));
}
