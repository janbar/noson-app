/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TOOLS_H
#define TOOLS_H

#include <QString>

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

#endif /* TOOLS_H */

