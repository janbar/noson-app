/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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
#ifndef PASOURCE_H
#define PASOURCE_H

#include "local_config.h"
#include "audiosource.h"

#include <pulse/simple.h>
#include <pulse/error.h>

namespace NSROOT
{

class PASourceWorker;

class PASource : public AudioSource
{
  friend class PASourceWorker;
public:
  PASource(const std::string& name, const std::string& deviceName);
  virtual ~PASource();

  bool open(OpenMode mode) override;
  void close() override;
  inline std::string getName() const override { return m_name; }
  std::string getDescription() const override { return m_deviceName; }
  AudioFormat getFormat() const override { return m_format; }

private:
  // Prevent copy
  PASource(const PASource& other);
  PASource& operator=(const PASource& other);

  std::string m_name;
  std::string m_deviceName;
  AudioFormat m_format;

  bool initPA();
  void freePA();

  int m_pa_error;
  pa_simple * m_pa;
  void(*m_blankKiller)(void*, int, int);

  PASourceWorker * m_p;
};

}

#endif /* PASOURCE_H */

