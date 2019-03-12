#ifndef TAHOSTYLE_H
#define TAHOSTYLE_H

///\class TahoStyle
///\brief Default drawing of buttons with pixmaps is not nice when buttons are in columns
/// we need to achieve left aligned pixmaps and left aligned text to them
/// Implementation based on https://stackoverflow.com/questions/44091339/qpushbutton-icon-aligned-left-with-text-centered

#include <QProxyStyle>

class TahoStyle : public QProxyStyle
{
    using super = QProxyStyle;
public:
    void drawControl(
            ControlElement element, const QStyleOption *opt,
            QPainter *p, const QWidget *widget = nullptr) const override;
};

#endif // TAHOSTYLE_H
