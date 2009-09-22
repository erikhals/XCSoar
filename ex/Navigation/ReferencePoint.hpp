/*
  Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000 - 2009

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


#ifndef REFERENCEPOINT_HPP
#define REFERENCEPOINT_HPP

#include "Util.h"

/////////////////////////////////////////////////
/// Abstract classes


/** Entity for objects that may be used as reference points for navigation */
class ReferencePoint {
public:
    ReferencePoint(const GEOPOINT & _location) : Location(_location) {
    };

  /** bearing from this to the reference
   */
    double bearing(const GEOPOINT & ref) const {
        return::Bearing(Location, ref);
    }

  /** distance from this to the reference
   */
    double distance(const GEOPOINT & ref) const {
        return::Distance(Location, ref);
    }

  /** bearing from this to the reference
   */
    double bearing(const ReferencePoint & ref) const {
        return::Bearing(Location, ref.getLocation());
    }

  /** distance from this to the reference
   */
    double distance(const ReferencePoint & ref) const {
        return::Distance(Location, ref.getLocation());
    }

  /** The actual location
   */
    const GEOPOINT & getLocation() const {
        return Location;
    };

protected:
    const GEOPOINT Location;
};


/////////////////////////////////////////////////
/// Abstract classes, types of task points










/////////////////////////////////////////////////
/// Concrete observation zones



///////



///////////////////////////////////////////////////////////

#endif
