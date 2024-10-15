#pragma execution_character_set("utf-8")

#include "CountCode.h"
#include <QApplication>
#pragma comment(linker,"/subsystem:windows /entry:mainCRTStartup")
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont font;
    font.setFamily("Microsoft Yahei");
    font.setPixelSize(13);
    a.setFont(font);

    CountCode w;
    w.setWindowTitle("代码行数统计（威桑：1428206861@qq.com）");
    w.show();

    return a.exec();
}
