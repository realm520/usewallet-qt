#pragma once
#include <QString>
#include <QMap>
class ErrorTranslator
{
public:
    static QString Translate(QString src);
	static ErrorTranslator& getErrorTranslator();

private:
    QMap<QString, QString> translate_map;
	ErrorTranslator();
};
