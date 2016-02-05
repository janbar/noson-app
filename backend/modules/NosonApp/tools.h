/*
 *      Copyright (C) 2016 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <cmath>
#include <QString>

namespace
{

  static inline int roundDouble(double value)
  {
    return (int)std::floor(value + 0.5);
  }

  static inline QString normalizedString(const QString& str)
  {
    QString ret;
    QString tmp = str.normalized(QString::NormalizationForm_D);
    ret.reserve(tmp.size());
    for (QString::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
      int cat = it->category();
      if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining)
        ret.append(*it);
    }
    return ret;
  }

}

#endif /* TOOLS_H */

