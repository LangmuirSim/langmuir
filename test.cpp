#include <QtCore>

//using namespace Langmuir;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QFile outFile("/home/adam/Desktop/out.bin");
    qDebug() << "file opens:" << outFile.open(QIODevice::WriteOnly);
    QDataStream out(&outFile);
    out.setVersion(QDataStream::Qt_4_7);
    out.setFloatingPointPrecision(QDataStream::DoublePrecision);
    out << qint64(10);
    outFile.close();

    QFile inFile("/home/adam/Desktop/out.bin");
    qDebug() << "file opens:" << inFile.open(QIODevice::ReadOnly);
    QDataStream in(&inFile);
    in.setVersion(QDataStream::Qt_4_7);
    in.setFloatingPointPrecision(QDataStream::DoublePrecision);
    qint64 value = 0;
    in >> value;
    inFile.close();
    qDebug() << "read value:" << value;
}
