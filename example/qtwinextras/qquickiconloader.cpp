/****************************************************************************
 **
 ** Copyright (C) 2022 Ivan Vizir <define-true-false@yandex.com>
 ** Copyright (C) 2022 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the QtWinExtras module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 3 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL3 included in the
 ** packaging of this file. Please review the following information to
 ** ensure the GNU Lesser General Public License version 3 requirements
 ** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 2.0 or (at your option) the GNU General
 ** Public license version 3 or any later version approved by the KDE Free
 ** Qt Foundation. The licenses are as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file. Please review the following
 ** information to ensure the GNU General Public License requirements will
 ** be met: https://www.gnu.org/licenses/gpl-2.0.html and
 ** https://www.gnu.org/licenses/gpl-3.0.html.
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "qquickiconloader_p.h"

#include <QtQuick/qquickimageprovider.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlfile.h>
#if QT_CONFIG(qml_network)
#  include <QtNetwork/qnetworkaccessmanager.h>
#  include <QtNetwork/qnetworkrequest.h>
#  include <QtNetwork/qnetworkreply.h>
#endif
#include <QtCore/qfileinfo.h>
#include <QtGui/qicon.h>
#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE

QVariant QQuickIconLoader::loadFromFile(const QUrl &url, const int metaTypeId)
{
    const QString path = QQmlFile::urlToLocalFileOrQrc(url);
    if (QFileInfo::exists(path)) {
        switch (metaTypeId) {
        case QMetaType::QIcon:
            return QVariant(QIcon(path));
        case QMetaType::QPixmap:
            return QVariant(QPixmap(path));
        default:
            qWarning("%s: Unsupported type: %d", Q_FUNC_INFO, metaTypeId);
            break;
        }
    }
    return QVariant();
}

#if QT_CONFIG(qml_network)
QNetworkReply *QQuickIconLoader::loadFromNetwork(const QUrl &url, const QQmlEngine *engine)
{
    return engine->networkAccessManager()->get(QNetworkRequest(url));
}
#endif // qml_network

QVariant QQuickIconLoader::loadFromImageProvider(const QUrl &url, const QQmlEngine *engine,
                                                 const int metaTypeId, const QSize &requestedSize)
{
    const QString providerId = url.host();
    const QString imageId = url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority).mid(1);
    QQuickImageProvider::ImageType imageType = QQuickImageProvider::Invalid;
    auto *provider = static_cast<QQuickImageProvider *>(engine->imageProvider(providerId));
    QSize size = {};
    QSize actualRequestedSize = requestedSize;
    if (!actualRequestedSize.isValid())
        actualRequestedSize = QSize(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
    if (provider)
        imageType = provider->imageType();
    QPixmap pixmap = {};
    switch (imageType) {
    case QQuickImageProvider::Image: {
        const QImage image = provider->requestImage(imageId, &size, actualRequestedSize);
        if (!image.isNull())
            pixmap = QPixmap::fromImage(image);
    } break;
    case QQuickImageProvider::Pixmap:
        pixmap = provider->requestPixmap(imageId, &size, actualRequestedSize);
        break;
    default:
        break;
    }
    if (!pixmap.isNull()) {
        switch (metaTypeId) {
        case QMetaType::QIcon:
            return QVariant(QIcon(pixmap));
        case QMetaType::QPixmap:
            return QVariant(pixmap);
        default:
            qWarning("%s: Unsupported type: %d", Q_FUNC_INFO, metaTypeId);
            break;
        }
    }
    return QVariant();
}

#if QT_CONFIG(qml_network)
QQuickIconLoaderNetworkReplyHandler::QQuickIconLoaderNetworkReplyHandler
    (QNetworkReply *reply, const int metaTypeId)
    : QObject(reply) , m_metaTypeId(metaTypeId)
{
    connect(reply, &QNetworkReply::finished, this, &QQuickIconLoaderNetworkReplyHandler::onRequestFinished);
}

void QQuickIconLoaderNetworkReplyHandler::onRequestFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    Q_ASSERT(reply);
    if (!reply) {
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << Q_FUNC_INFO << reply->url() << "failed:" << reply->errorString();
        return;
    }
    const QByteArray data = reply->readAll();
    QPixmap pixmap = {};
    if (pixmap.loadFromData(data)) {
        switch (m_metaTypeId) {
        case QMetaType::QIcon:
            Q_EMIT finished(QVariant(QIcon(pixmap)));
            break;
        case QMetaType::QPixmap:
            Q_EMIT finished(QVariant(pixmap));
            break;
        default:
            qWarning("%s: Unsupported type: %d", Q_FUNC_INFO, m_metaTypeId);
            break;
        }
    }
    reply->deleteLater();
}
#endif // qml_network

QT_END_NAMESPACE
