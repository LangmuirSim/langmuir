#include <QtCore>

#include <iostream>
#include <fstream>
#include <boost/random.hpp>
#include <ctime>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    boost::mt19937 rand_1(static_cast <quint64>(time(0)));
    boost::uniform_01<qreal> dist_1;
    boost::variate_generator<boost::mt19937 &, boost::uniform_01<qreal> > gen_1(rand_1, dist_1);

    boost::mt19937 rand_2(static_cast <quint64>(100));
    boost::uniform_01<qreal> dist_2;
    boost::variate_generator<boost::mt19937 &, boost::uniform_01<qreal> > gen_2(rand_2, dist_2);

    qDebug() << "rand_1 == rand_2" << (rand_1 == rand_2);
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));

    qint64 offset = 0;
    const char * name = "/home/adam/Desktop/test.dat";

    //Write some stuff
    {
        QFile file;
        file.setFileName(name);
        if (file.exists()) file.remove();
        file.open(QIODevice::ReadWrite);
        QDataStream stream(&file);
        stream << quint64(10001);
        file.flush();
        offset = file.pos();
    }

    {
        fstream stream(name);
        stream.seekp(offset);
        stream << rand_1;
        stream.flush();
        stream.close();
    }

    //Read some stuff
    {
        QFile file;
        file.setFileName(name);
        file.open(QIODevice::ReadWrite);
        QDataStream stream(&file);
        quint64 value;
        stream >> value;
        offset = file.pos();
    }

    {
        ifstream stream(name);
        stream.seekg(offset);
        stream >> rand_2;
        stream.close();
    }

    qDebug() << "";
    qDebug() << "rand_1 == rand_2" << (rand_1 == rand_2);
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
    qDebug() << qPrintable(QString("gen_1() %1 gen_2 %2").arg(gen_1(),10,'e',7).arg(gen_2(),10,'e',7));
}
