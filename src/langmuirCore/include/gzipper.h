#ifndef GZIPPER_H
#define GZIPPER_H

#include <QString>

/**
 * @brief gunzip a file using QProcess
 * @param fileName name of file to gunzip
 * @param msecs timeout time
 * @return altered file name
 */
QString gunzip(QString fileName, bool *wasZipped = NULL);

/**
 * @brief gzip a file using QProcess
 * @param fileName name of file to gzip
 * @param msecs timeout time
 * @return altered file name
 */
QString gzip (QString fileName);

#endif // GZIPPER_H
