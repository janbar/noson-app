/*
 *      Copyright (C) 2018-2019 Jean-Luc Barriere
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

#include "alarmsmodel.h"
#include "sonos.h"

using namespace nosonapp;

AlarmItem::AlarmItem(const SONOS::AlarmPtr& ptr)
: m_ptr(ptr)
{
  if (ptr)
    m_valid = true;
}

QVariant AlarmItem::payload() const
{
  QVariant var;
  var.setValue<SONOS::AlarmPtr>(m_ptr);
  return var;
}

QString AlarmItem::id() const
{
  return QString::fromUtf8(m_ptr->GetId().c_str());
}

bool AlarmItem::enabled() const
{
  return m_ptr->GetEnabled();
}

void AlarmItem::setEnabled(bool value)
{
  m_ptr->SetEnabled(value);
}

QString AlarmItem::programURI() const
{
  return QString::fromUtf8(m_ptr->GetProgramURI().c_str());
}

void AlarmItem::setProgramURI(const QString& value)
{
  m_ptr->SetProgramURI(value.toUtf8().constData());
}

QString AlarmItem::programTitle() const
{
  if (m_ptr->GetProgramMetadata())
    return QString::fromUtf8(m_ptr->GetProgramMetadata()->GetValue("dc:title").c_str());
  return "";
}

QVariant AlarmItem::programMetadata() const
{
  QVariant var;
  var.setValue<SONOS::DigitalItemPtr>(m_ptr->GetProgramMetadata());
  return var;
}

void AlarmItem::setProgramMetadata(const QVariant& value)
{
  m_ptr->SetProgramMetadata(value.value<SONOS::DigitalItemPtr>());
}

QString AlarmItem::playMode() const
{
  return QString::fromUtf8(m_ptr->GetPlayMode().c_str());
}

void AlarmItem::setPlayMode(const QString& value)
{
  if (value == SONOS::PlayModeTable[SONOS::PlayMode_SHUFFLE])
    m_ptr->SetPlayMode(SONOS::PlayMode_SHUFFLE);
  else if (value == SONOS::PlayModeTable[SONOS::PlayMode_SHUFFLE_NOREPEAT])
    m_ptr->SetPlayMode(SONOS::PlayMode_SHUFFLE_NOREPEAT);
  else if (value == SONOS::PlayModeTable[SONOS::PlayMode_REPEAT_ALL])
    m_ptr->SetPlayMode(SONOS::PlayMode_REPEAT_ALL);
  else
    m_ptr->SetPlayMode(SONOS::PlayMode_NORMAL);
}

int AlarmItem::volume() const {
  return m_ptr->GetVolume();
}

void AlarmItem::setVolume(int value)
{
  m_ptr->SetVolume(value);
}

bool AlarmItem::includeLinkedZones() const
{
  return m_ptr->GetIncludeLinkedZones();
}

void AlarmItem::setIncludeLinkedZones(bool value)
{
  m_ptr->SetIncludeLinkedZones(value);
}

QString AlarmItem::roomUUID() const
{
  return QString::fromUtf8(m_ptr->GetRoomUUID().c_str());
}

void AlarmItem::setRoomUUID(const QString& value)
{
  m_ptr->SetRoomUUID(value.toUtf8().constData());
}

QString AlarmItem::startLocalTime() const
{
  return QString::fromUtf8(m_ptr->GetStartLocalTime().c_str());
}

void AlarmItem::setStartLocalTime(const QString& value)
{
  m_ptr->SetStartLocalTime(value.toUtf8().constData());
}

QString AlarmItem::duration() const
{
  return QString::fromUtf8(m_ptr->GetDuration().c_str());
}

void AlarmItem::setDuration(const QString& value)
{
  m_ptr->SetDuration(value.toUtf8().constData());
}

QString AlarmItem::recurrence() const
{
  return QString::fromUtf8(m_ptr->GetRecurrence().c_str());
}

void AlarmItem::setRecurrence(const QString& value)
{
  m_ptr->SetRecurrence(value.toUtf8().constData());
}

AlarmsModel::AlarmsModel(QObject* parent)
: QAbstractListModel(parent)
{
}

AlarmsModel::~AlarmsModel()
{
  qDeleteAll(m_data);
  m_data.clear();
  qDeleteAll(m_items);
  m_items.clear();
}

void AlarmsModel::addItem(AlarmItem* item)
{
  {
    LockGuard g(m_lock);
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items << item;
    endInsertRows();
  }
  emit countChanged();
}

int AlarmsModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
#ifdef USE_RECURSIVE_MUTEX
  LockGuard g(m_lock);
#endif
  return m_items.count();
}

QVariant AlarmsModel::data(const QModelIndex& index, int role) const
{
#ifdef USE_RECURSIVE_MUTEX
  LockGuard g(m_lock);
#endif
  if (index.row() < 0 || index.row() >= m_items.count())
      return QVariant();

  const AlarmItem* item = m_items[index.row()];
  switch (role)
  {
  case PayloadRole:
    return item->payload();
  case IdRole:
    return item->id();
  case EnabledRole:
    return item->enabled();
  case ProgramUriRole:
    return item->programURI();
  case ProgramTitleRole:
    return item->programTitle();
  case ProgramMetadataRole:
    return item->programMetadata();
  case PlayModeRole:
    return item->playMode();
  case VolumeRole:
    return item->volume();
  case IncludeLinkedZonesRole:
    return item->includeLinkedZones();
  case RoomIdRole:
    return item->roomUUID();
  case StartLocalTimeRole:
    return item->startLocalTime();
  case DurationRole:
    return item->duration();
  case RecurrenceRole:
    return item->recurrence();
  default:
    return QVariant();
  }
}

bool AlarmsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  LockGuard g(m_lock);
  if (index.row() < 0 || index.row() >= m_items.count())
      return false;

  AlarmItem* item = m_items[index.row()];
  switch (role)
  {
  case EnabledRole:
    item->setEnabled(value.toBool());
    break;
  case IncludeLinkedZonesRole:
    item->setIncludeLinkedZones(value.toBool());
    break;
  case ProgramUriRole:
    item->setProgramURI(value.toString());
    break;
  case ProgramMetadataRole:
    item->setProgramMetadata(value);
    break;
  case PlayModeRole:
    item->setPlayMode(value.toString());
    break;
  case VolumeRole:
    item->setVolume(value.toInt());
    break;
  case RoomIdRole:
    item->setRoomUUID(value.toString());
    break;
  case StartLocalTimeRole:
    item->setStartLocalTime(value.toString());
    break;
  case DurationRole:
    item->setDuration(value.toString());
    break;
  case RecurrenceRole:
    item->setRecurrence(value.toString());
    break;
  case IdRole:
  case PayloadRole:
  case ProgramTitleRole:
  default:
    return false;
  }
  emit dataChanged(index, index);
  return true;
}

bool AlarmsModel::insertRow(int row, const QModelIndex& parent)
{
  Q_UNUSED(parent);
  {
    LockGuard g(m_lock);
    if (row < 0 || row > m_items.count())
      return false;
    SONOS::AlarmPtr ptr(new SONOS::Alarm());
    beginInsertRows(QModelIndex(), row, row);
    m_items.insert(row, new AlarmItem(ptr));
    endInsertRows();
  }
  emit countChanged();
  return true;
}

bool AlarmsModel::removeRow(int row, const QModelIndex& parent)
{
  Q_UNUSED(parent);
  {
    LockGuard g(m_lock);
    if (row < 0 || row >= m_items.count())
      return false;
    beginRemoveRows(QModelIndex(), row, row);
    delete m_items.at(row);
    m_items.removeAt(row);
    endRemoveRows();
  }
  emit countChanged();
  return true;
}

int AlarmsModel::append()
{
  int row = m_items.count();
  if (insertRow(row))
    return row;
  return -1;
}

bool AlarmsModel::remove(int row)
{
  return removeRow(row);
}

QHash<int, QByteArray> AlarmsModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[PayloadRole] = "payload";
  roles[IdRole] = "id";
  roles[EnabledRole] = "enabled";
  roles[ProgramUriRole] = "programUri";
  roles[ProgramTitleRole] = "programTitle";
  roles[ProgramMetadataRole] = "programMetadata";
  roles[PlayModeRole] = "playRole";
  roles[VolumeRole] = "volume";
  roles[IncludeLinkedZonesRole] = "includeLinkedZones";
  roles[RoomIdRole] = "roomId";
  roles[StartLocalTimeRole] = "startLocalTime";
  roles[DurationRole] = "duration";
  roles[RecurrenceRole] = "recurrence";

  return roles;
}

QVariantMap AlarmsModel::get(int row)
{
  LockGuard g(m_lock);
  if (row < 0 || row >= m_items.count())
    return QVariantMap();
  const AlarmItem* item = m_items[row];
  QVariantMap model;
  QHash<int, QByteArray> roles = roleNames();
  model[roles[PayloadRole]] = item->payload();
  model[roles[IdRole]] = item->id();
  model[roles[EnabledRole]] = item->enabled();
  model[roles[ProgramUriRole]] = item->programURI();
  model[roles[ProgramTitleRole]] = item->programTitle();
  model[roles[ProgramMetadataRole]] = item->programMetadata();
  model[roles[PlayModeRole]] = item->playMode();
  model[roles[VolumeRole]] = item->volume();
  model[roles[IncludeLinkedZonesRole]] = item->includeLinkedZones();
  model[roles[RoomIdRole]] = item->roomUUID();
  model[roles[StartLocalTimeRole]] = item->startLocalTime();
  model[roles[DurationRole]] = item->duration();
  model[roles[RecurrenceRole]] = item->recurrence();
  return model;
}

void AlarmsModel::clearData()
{
  LockGuard g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
}

bool AlarmsModel::loadData()
{
  setUpdateSignaled(false);

  if (!m_provider)
  {
    emit loaded(false);
    return false;
  }

  LockGuard g(m_lock);
  qDeleteAll(m_data);
  m_data.clear();
  m_dataState = DataStatus::DataNotFound;
  SONOS::AlarmList alarms = m_provider->getSystem().GetAlarmList();
  for (SONOS::AlarmList::iterator it = alarms.begin(); it != alarms.end(); ++it)
  {
    AlarmItem* item = new AlarmItem(*it);
    if (item->isValid())
      m_data << item;
    else
      delete item;
  }
  m_dataState = DataStatus::DataLoaded;
  emit loaded(true);
  return true;
}

bool AlarmsModel::asyncLoad()
{
  if (m_provider)
  {
    m_provider->runContentLoader(this);
    return true;
  }
  return false;
}

void AlarmsModel::resetModel()
{
  {
    LockGuard g(m_lock);
    if (m_dataState != DataStatus::DataLoaded)
      return;
    beginResetModel();
    if (m_items.count() > 0)
    {
      beginRemoveRows(QModelIndex(), 0, m_items.count()-1);
      qDeleteAll(m_items);
      m_items.clear();
      endRemoveRows();
    }
    if (m_data.count() > 0)
    {
      beginInsertRows(QModelIndex(), 0, m_data.count()-1);
      foreach (AlarmItem* item, m_data)
          m_items << item;
      m_data.clear();
      endInsertRows();
    }
    m_dataState = DataStatus::DataSynced;
    endResetModel();
  }
  emit countChanged();
}

void AlarmsModel::handleDataUpdate()
{
  if (!updateSignaled())
  {
    setUpdateSignaled(true);
    dataUpdated();
  }
}
