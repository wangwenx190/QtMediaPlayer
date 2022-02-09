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

#pragma once

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtQml/qqmlregistration.h>

class QSettings;

class Settings : public QObject
{
    Q_OBJECT
#ifdef QML_ELEMENT
    QML_ELEMENT
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_DISABLE_COPY_MOVE(Settings)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged FINAL)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
    Q_PROPERTY(QUrl file READ file WRITE setFile NOTIFY fileChanged FINAL)
    Q_PROPERTY(bool hardwareDecoding READ hardwareDecoding WRITE setHardwareDecoding NOTIFY hardwareDecodingChanged FINAL)
    Q_PROPERTY(QUrl dir READ dir WRITE setDir NOTIFY dirChanged FINAL)
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged FINAL)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged FINAL)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged FINAL)
    Q_PROPERTY(QVariant logLevel READ logLevel WRITE setLogLevel NOTIFY logLevelChanged FINAL)
    Q_PROPERTY(bool restoreLastWindowGeometry READ restoreLastWindowGeometry WRITE setRestoreLastWindowGeometry NOTIFY restoreLastWindowGeometryChanged FINAL)
    Q_PROPERTY(bool startFromLastPosition READ startFromLastPosition WRITE setStartFromLastPosition NOTIFY startFromLastPositionChanged FINAL)
    Q_PROPERTY(bool saveHistory READ saveHistory WRITE setSaveHistory NOTIFY saveHistoryChanged FINAL)
    Q_PROPERTY(QVariant windowState READ windowState WRITE setWindowState NOTIFY windowStateChanged FINAL)
    Q_PROPERTY(bool pauseWhenMinimized READ pauseWhenMinimized WRITE setPauseWhenMinimized NOTIFY pauseWhenMinimizedChanged FINAL)
    Q_PROPERTY(bool enableTimelinePreview READ enableTimelinePreview WRITE setEnableTimelinePreview NOTIFY enableTimelinePreviewChanged FINAL)
    Q_PROPERTY(qreal timelinePreviewZoomFactor READ timelinePreviewZoomFactor WRITE setTimelinePreviewZoomFactor NOTIFY timelinePreviewZoomFactorChanged FINAL)
    Q_PROPERTY(bool osdShowLocalTime READ osdShowLocalTime WRITE setOsdShowLocalTime NOTIFY osdShowLocalTimeChanged FINAL)
    Q_PROPERTY(QString snapshotFormat READ snapshotFormat WRITE setSnapshotFormat NOTIFY snapshotFormatChanged FINAL)
    Q_PROPERTY(QUrl snapshotDirectory READ snapshotDirectory WRITE setSnapshotDirectory NOTIFY snapshotDirectoryChanged FINAL)

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override;

    [[nodiscard]] bool mute() const;
    void setMute(const bool value);

    [[nodiscard]] qreal volume() const;
    void setVolume(const qreal value);

    [[nodiscard]] QUrl file() const;
    void setFile(const QUrl &value);

    [[nodiscard]] bool hardwareDecoding() const;
    void setHardwareDecoding(const bool value);

    [[nodiscard]] QUrl dir() const;
    void setDir(const QUrl &value);

    [[nodiscard]] qreal x() const;
    void setX(const qreal value);

    [[nodiscard]] qreal y() const;
    void setY(const qreal value);

    [[nodiscard]] qreal width() const;
    void setWidth(const qreal value);

    [[nodiscard]] qreal height() const;
    void setHeight(const qreal value);

    [[nodiscard]] QVariant logLevel() const;
    void setLogLevel(const QVariant &value);

    [[nodiscard]] bool restoreLastWindowGeometry() const;
    void setRestoreLastWindowGeometry(const bool value);

    [[nodiscard]] bool startFromLastPosition() const;
    void setStartFromLastPosition(const bool value);

    [[nodiscard]] bool saveHistory() const;
    void setSaveHistory(const bool value);

    [[nodiscard]] QVariant windowState() const;
    void setWindowState(const QVariant &value);

    [[nodiscard]] bool pauseWhenMinimized() const;
    void setPauseWhenMinimized(const bool value);

    [[nodiscard]] bool enableTimelinePreview() const;
    void setEnableTimelinePreview(const bool value);

    [[nodiscard]] qreal timelinePreviewZoomFactor() const;
    void setTimelinePreviewZoomFactor(const qreal value);

    [[nodiscard]] bool osdShowLocalTime() const;
    void setOsdShowLocalTime(const bool value);

    [[nodiscard]] QString snapshotFormat() const;
    void setSnapshotFormat(const QString &value);

    [[nodiscard]] QUrl snapshotDirectory() const;
    void setSnapshotDirectory(const QUrl &value);

Q_SIGNALS:
    void muteChanged();
    void volumeChanged();
    void fileChanged();
    void hardwareDecodingChanged();
    void dirChanged();
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void logLevelChanged();
    void restoreLastWindowGeometryChanged();
    void startFromLastPositionChanged();
    void saveHistoryChanged();
    void windowStateChanged();
    void pauseWhenMinimizedChanged();
    void enableTimelinePreviewChanged();
    void timelinePreviewZoomFactorChanged();
    void osdShowLocalTimeChanged();
    void snapshotFormatChanged();
    void snapshotDirectoryChanged();

private:
    QScopedPointer<QSettings> m_settings;
};
