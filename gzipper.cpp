#include "gzipper.h"
#include <QStringList>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QDebug>

QString gunzip(QString fileName, int msecs)
{
    QFileInfo info(fileName);
    if (!info.exists())
    {
        qDebug("langmuir: %s does not exist", qPrintable(info.fileName()));
        return info.fileName();
    }

    if (info.suffix() != "gz")
    {
        qDebug("langmuir: %s is not gzipped", qPrintable(info.fileName()));
        return info.fileName();
    }

    QProcess proc;
    qDebug("langmuir: uncompressing %s", qPrintable(info.fileName()));
    proc.start("gunzip", QStringList() << fileName);

    if (!proc.waitForStarted(msecs))
    {
        qFatal("langmuir: QProcess took too long to start");
    }

    if (!proc.waitForFinished(msecs))
    {
        qFatal("langmuir: QProcess took too long to finish");
    }

    if (proc.exitStatus() != QProcess::NormalExit)
    {
        qFatal("langmuir: QProcess has failed (%d)", proc.exitCode());
    }

    qDebug("langmuir: uncompression complete");
    return info.completeBaseName();
}

QString gzip (QString fileName, int msecs)
{
    QFileInfo info(fileName);
    if (!info.exists())
    {
        qDebug("langmuir: %s does not exist", qPrintable(info.fileName()));
        return info.fileName();
    }

    if (info.suffix() == "gz")
    {
        qDebug("langmuir: %s is already gzipped", qPrintable(info.fileName()));
        return info.fileName();
    }

    QProcess proc;
    qDebug("langmuir: compressing %s", qPrintable(info.fileName()));
    proc.start("gzip", QStringList() << fileName);

    if (!proc.waitForStarted(msecs))
    {
        qFatal("langmuir: QProcess took too long to start");
    }

    if (!proc.waitForFinished(msecs))
    {
        qFatal("langmuir: QProcess took too long to finish");
    }

    if (proc.exitStatus() != QProcess::NormalExit)
    {
        qFatal("langmuir: QProcess has failed (%d)", proc.exitCode());
    }

    qDebug("langmuir: compression complete");
    return info.fileName().append(".gz");
}
