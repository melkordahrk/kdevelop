#include <QHash>
#include <QString>
int main()
{
    QHash<QString, QString> h;
    h["10"] = "100";
    h["20"] = "200";
    h["30"] = "300";
    return 0;
}
