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
#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include "mediafile.h"

#include <QMap>
#include <QSharedPointer>

namespace mediascanner
{

class Model
{
public:
  Model(const MediaFilePtr& file) : m_file(file) { }
  virtual ~Model() { }
  virtual const QByteArray& key() const = 0;
protected:
  MediaFilePtr m_file;
};

template <class T>
struct Tuple
{
  static_assert(std::is_base_of<Model, T>::value, "T must derive from Model");
  T model;
  QMap<unsigned, MediaFilePtr> files;
  Tuple(const T& m) : model(m) { }
};

template<class T>
class Aggregate : public QMap<QByteArray, QSharedPointer< Tuple<T> > >
{
public:
  Aggregate() { }
  virtual ~Aggregate() { }

  typedef QSharedPointer< Tuple<T> > TuplePtr;
  typedef QMap<QByteArray, TuplePtr> KeyMap;
  typedef typename KeyMap::iterator iterator;

  inline bool insertFile(const MediaFilePtr& file, QByteArray * key)
  {
    bool ng = false;
    T model(file);
    iterator it = KeyMap::find(model.key());
    if (it == KeyMap::end())
    {
      ng = true;
      it = KeyMap::insert(model.key(), TuplePtr(new Tuple<T>(model)));
    }
    if (key)
      *key = model.key();
    it.value()->files.insert(file->fileId, file);
    return ng;
  }

  inline bool removeFile(const MediaFilePtr& file, QByteArray * key)
  {
    T model(file);
    iterator it = KeyMap::find(model.key());
    if (it != KeyMap::end())
    {
      it.value()->files.remove(file->fileId);
      if (key)
        *key = model.key();
      if (it.value()->files.size() == 0)
      {
        KeyMap::erase(it);
        return true;
      }
    }
    return false;
  }

};

}

#endif /* AGGREGATOR_H */

