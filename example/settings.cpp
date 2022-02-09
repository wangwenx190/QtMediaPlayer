/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "settings.h"
#include <QtCore/qcoreapplication.h>
#include <QtCore/qsettings.h>
#include <QtGui/qwindow.h>

static const QString kRestoreLastWindowGeometry = QStringLiteral("window/restoreLastWindowGeometry");
static const QString kX = QStringLiteral("window/x");
static const QString kY = QStringLiteral("window/y");
static const QString kWidth = QStringLiteral("window/width");
static const QString kHeight = QStringLiteral("window/height");
static const QString kWindowState = QStringLiteral("window/windowState");
static const QString kMute = QStringLiteral("player/mute");
static const QString kVolume = QStringLiteral("player/volume");
static const QString kFile = QStringLiteral("player/file");
static const QString kHardwareDecoding = QStringLiteral("player/hardwareDecoding");
static const QString kDir = QStringLiteral("player/dir");
static const QString kLogLevel = QStringLiteral("player/logLevel");
static const QString kStartFromLastPosition = QStringLiteral("player/startFromLastPosition");
static const QString kSaveHistory = QStringLiteral("player/saveHistory");
static const QString kPauseWhenMinimized = QStringLiteral("player/pauseWhenMinimized");
static const QString kEnableTimelinePreview = QStringLiteral("player/enableTimelinePreview");
static const QString kTimelinePreviewZoomFactor = QStringLiteral("player/timelinePreviewZoomFactor");
static const QString kOsdShowLocalTime = QStringLiteral("player/osdShowLocalTime");
static const QString kSnapshotFormat = QStringLiteral("player/snapshotFormat");
static const QString kSnapshotDirectory = QStringLiteral("player/snapshotDirectory");

Settings::Settings(QObject *parent) : QObject(parent)
{
    static const QString iniFilePath = QCoreApplication::applicationDirPath() + QStringLiteral("/settings.ini");
    m_settings.reset(new QSettings(iniFilePath, QSettings::IniFormat, this));
}

Settings::~Settings() = default;

bool Settings::mute() const
{
    return m_settings->value(kMute, false).toBool();
}

void Settings::setMute(const bool value)
{
    if (mute() == value) {
        return;
    }
    m_settings->setValue(kMute, value);
    Q_EMIT muteChanged();
}

qreal Settings::volume() const
{
    return m_settings->value(kVolume, 1.0).toReal();
}

void Settings::setVolume(const qreal value)
{
    if (volume() == value) {
        return;
    }
    m_settings->setValue(kVolume, value);
    Q_EMIT volumeChanged();
}

QUrl Settings::file() const
{
    return m_settings->value(kFile).toUrl();
}

void Settings::setFile(const QUrl &value)
{
    if (file() == value) {
        return;
    }
    m_settings->setValue(kFile, value);
    Q_EMIT fileChanged();
}

bool Settings::hardwareDecoding() const
{
    return m_settings->value(kHardwareDecoding, false).toBool();
}

void Settings::setHardwareDecoding(const bool value)
{
    if (hardwareDecoding() == value) {
        return;
    }
    m_settings->setValue(kHardwareDecoding, value);
    Q_EMIT hardwareDecodingChanged();
}

QUrl Settings::dir() const
{
    return m_settings->value(kDir).toUrl();
}

void Settings::setDir(const QUrl &value)
{
    if (dir() == value) {
        return;
    }
    m_settings->setValue(kDir, value);
    Q_EMIT dirChanged();
}

qreal Settings::x() const
{
    return m_settings->value(kX).toReal();
}

void Settings::setX(const qreal value)
{
    if (x() == value) {
        return;
    }
    m_settings->setValue(kX, value);
    Q_EMIT xChanged();
}

qreal Settings::y() const
{
    return m_settings->value(kY).toReal();
}

void Settings::setY(const qreal value)
{
    if (y() == value) {
        return;
    }
    m_settings->setValue(kY, value);
    Q_EMIT yChanged();
}

qreal Settings::width() const
{
    return m_settings->value(kWidth).toReal();
}

void Settings::setWidth(const qreal value)
{
    if (width() == value) {
        return;
    }
    m_settings->setValue(kWidth, value);
    Q_EMIT widthChanged();
}

qreal Settings::height() const
{
    return m_settings->value(kHeight).toReal();
}

void Settings::setHeight(const qreal value)
{
    if (height() == value) {
        return;
    }
    m_settings->setValue(kHeight, value);
    Q_EMIT heightChanged();
}

QVariant Settings::logLevel() const
{
    return m_settings->value(kLogLevel);
}

void Settings::setLogLevel(const QVariant &value)
{
    if (logLevel() == value) {
        return;
    }
    m_settings->setValue(kLogLevel, value);
    Q_EMIT logLevelChanged();
}

bool Settings::restoreLastWindowGeometry() const
{
    return m_settings->value(kRestoreLastWindowGeometry, false).toBool();
}

void Settings::setRestoreLastWindowGeometry(const bool value)
{
    if (restoreLastWindowGeometry() == value) {
        return;
    }
    m_settings->setValue(kRestoreLastWindowGeometry, value);
    Q_EMIT restoreLastWindowGeometryChanged();
}

bool Settings::startFromLastPosition() const
{
    return m_settings->value(kStartFromLastPosition, false).toBool();
}

void Settings::setStartFromLastPosition(const bool value)
{
    if (startFromLastPosition() == value) {
        return;
    }
    m_settings->setValue(kStartFromLastPosition, value);
    Q_EMIT startFromLastPositionChanged();
}

bool Settings::saveHistory() const
{
    return m_settings->value(kSaveHistory, false).toBool();
}

void Settings::setSaveHistory(const bool value)
{
    if (saveHistory() == value) {
        return;
    }
    m_settings->setValue(kSaveHistory, value);
    Q_EMIT saveHistoryChanged();
}

QVariant Settings::windowState() const
{
    return m_settings->value(kWindowState, QVariant::fromValue(QWindow::Windowed));
}

void Settings::setWindowState(const QVariant &value)
{
    if (windowState() == value) {
        return;
    }
    m_settings->setValue(kWindowState, value);
    Q_EMIT windowStateChanged();
}

bool Settings::pauseWhenMinimized() const
{
    return m_settings->value(kPauseWhenMinimized, true).toBool();
}

void Settings::setPauseWhenMinimized(const bool value)
{
    if (pauseWhenMinimized() == value) {
        return;
    }
    m_settings->setValue(kPauseWhenMinimized, value);
    Q_EMIT pauseWhenMinimizedChanged();
}

bool Settings::enableTimelinePreview() const
{
    return m_settings->value(kEnableTimelinePreview, true).toBool();
}

void Settings::setEnableTimelinePreview(const bool value)
{
    if (enableTimelinePreview() == value) {
        return;
    }
    m_settings->setValue(kEnableTimelinePreview, value);
    Q_EMIT enableTimelinePreviewChanged();
}

qreal Settings::timelinePreviewZoomFactor() const
{
    return m_settings->value(kTimelinePreviewZoomFactor, 0.2).toReal();
}

void Settings::setTimelinePreviewZoomFactor(const qreal value)
{
    if (timelinePreviewZoomFactor() == value) {
        return;
    }
    m_settings->setValue(kTimelinePreviewZoomFactor, value);
    Q_EMIT timelinePreviewZoomFactorChanged();
}

bool Settings::osdShowLocalTime() const
{
    return m_settings->value(kOsdShowLocalTime, true).toBool();
}

void Settings::setOsdShowLocalTime(const bool value)
{
    if (osdShowLocalTime() == value) {
        return;
    }
    m_settings->setValue(kOsdShowLocalTime, value);
    Q_EMIT osdShowLocalTimeChanged();
}

QString Settings::snapshotFormat() const
{
    return m_settings->value(kSnapshotFormat, QStringLiteral("png")).toString();
}

void Settings::setSnapshotFormat(const QString &value)
{
    if (snapshotFormat() == value) {
        return;
    }
    m_settings->setValue(kSnapshotFormat, value);
    Q_EMIT snapshotFormatChanged();
}

QUrl Settings::snapshotDirectory() const
{
    return m_settings->value(kSnapshotDirectory).toUrl();
}

void Settings::setSnapshotDirectory(const QUrl &value)
{
    if (snapshotDirectory() == value) {
        return;
    }
    m_settings->setValue(kSnapshotDirectory, value);
    Q_EMIT snapshotDirectoryChanged();
}
