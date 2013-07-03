#include "gzipper.h"
#include <QStringList>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QDebug>

QString gunzip(QString fileName, bool *wasZipped)
{
    if (wasZipped) { *wasZipped = false; }

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
    qDebug("langmuir: decompressing %s", qPrintable(info.fileName()));
    proc.start("gunzip", QStringList() << fileName);

    if (!proc.waitForStarted(300000))
    {
        qFatal("langmuir: QProcess took too long to start");
    }

    if (!proc.waitForFinished(300000))
    {
        qFatal("langmuir: QProcess took too long to finish");
    }

    if (proc.exitStatus() != QProcess::NormalExit)
    {
        qFatal("langmuir: QProcess has failed (%d)", proc.exitCode());
    }

    qDebug("langmuir: decompression complete");

    if (wasZipped) { *wasZipped = true; }
    return info.completeBaseName();
}

QString gzip (QString fileName)
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

    if (!proc.waitForStarted(300000))
    {
        qFatal("langmuir: QProcess took too long to start");
    }

    if (!proc.waitForFinished(300000))
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
