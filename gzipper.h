#ifndef GZIPPER_H
#define GZIPPER_H

#include <QString>

/**
 * @brief gunzip a file using QProcess
 * @param fileName name of file to gunzip
 * @param msecs timeout time
 * @return altered file name
 */
QString gunzip(QString fileName, int msecs=300000);

/**
 * @brief gzip a file using QProcess
 * @param fileName name of file to gzip
 * @param msecs timeout time
 * @return altered file name
 */
QString gzip (QString fileName, int msecs=300000);

#endif // GZIPPER_H
