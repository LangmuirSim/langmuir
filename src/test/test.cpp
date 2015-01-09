#include <QCoreApplication>
#include <QDebug>

#include "nodefileparser.h"
using namespace LangmuirCore;

int main (int argc, char *argv[])
{
    QString npath = "/home/adam/Desktop/nodefile";
    QString gpath = "/home/adam/Desktop/gpufile";

    //NodeFileParser nfp(npath, gpath);
    //NodeFileParser nfp(npath, "");
    //NodeFileParser nfp("", gpath);
    //NodeFileParser nfp("", "");
    NodeFileParser nfp;
    nfp.clear();
    nfp.createNode("nodeA", 4);
    nfp.createNode("nodeB", 4);
    qDebug() << nfp;
}
