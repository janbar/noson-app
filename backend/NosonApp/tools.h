/*
 *      Copyright (C) 2016-2019 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Noson.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NOSONAPPTOOLS_H
#define NOSONAPPTOOLS_H

#include <cmath>
#include <QString>

namespace nosonapp
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
    int pcat = QChar::Separator_Space;
    for (QString::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
      int cat = it->category();
      if (cat != QChar::Mark_NonSpacing && cat != QChar::Mark_SpacingCombining)
      {
        if (cat != QChar::Separator_Space || pcat != QChar::Separator_Space)
          ret.append(*it);
        pcat = cat;
      }
    }
    if (!ret.isEmpty() && pcat == QChar::Separator_Space)
      ret.truncate(ret.length() - 1);
    return ret;
  }

}

#endif /* NOSONAPPTOOLS_H */

