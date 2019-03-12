#include "tahostyle.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QWidget>

// Copied from Qt source code..
static QWindow *qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : nullptr;
}

void TahoStyle::drawControl(
        QStyle::ControlElement element, const QStyleOption *opt,
        QPainter *p, const QWidget *widget) const
{
    if(element==CE_PushButtonLabel)
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(opt))
        {
                QRect textRect = button->rect;
                uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                    tf |= Qt::TextHideMnemonic;

                if (!button->icon.isNull()) {
                    QRect iconRect;
                    QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                    if (mode == QIcon::Normal && button->state & State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (button->state & State_On)
                        state = QIcon::On;

                    QPixmap pixmap = button->icon.pixmap(qt_getWindow(widget), button->iconSize, mode, state);

                    const auto pixmapWidth = qRound(pixmap.width() / pixmap.devicePixelRatio());
                    const auto pixmapHeight = qRound(pixmap.height() / pixmap.devicePixelRatio());
                    auto labelWidth = pixmapWidth;
                    const auto labelHeight = pixmapHeight;
                    const auto iconSpacing = 4;//### 4 is currently hardcoded in QPushButton::sizeHint()
                    const auto textWidth = button->fontMetrics.boundingRect(opt->rect, tf, button->text).width();
                    if (!button->text.isEmpty())
                        labelWidth += (textWidth + iconSpacing);

                    /*************************************************************/
                    // Make the icon rectangle always be 10px in from the left edge
                    /*************************************************************/
                    const auto iconPixelOffset = 10;
                    iconRect = QRect(iconPixelOffset,
                                     textRect.y() + (textRect.height() - labelHeight) / 2,
                                     pixmapWidth, pixmapHeight);

                    iconRect = visualRect(button->direction, textRect, iconRect);

                    /***********************************/
                    // Always horizontal align the text
                    /***********************************/
                    tf |= Qt::AlignLeft;
                    // let have text the same offset from icon as icon from left button side
                    textRect.setLeft(iconRect.right() + iconPixelOffset);

                    if (button->state & (State_On | State_Sunken))
                        iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                                           proxy()->pixelMetric(PM_ButtonShiftVertical, opt, widget));
                    p->drawPixmap(iconRect, pixmap);
                } else {
                    tf |= Qt::AlignHCenter;
                }
                if (button->state & (State_On | State_Sunken))
                    textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                                 proxy()->pixelMetric(PM_ButtonShiftVertical, opt, widget));

                if (button->features & QStyleOptionButton::HasMenu) {
                    int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
                    if (button->direction == Qt::LeftToRight)
                        textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
                    else
                        textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
                }
                proxy()->drawItemText(p, textRect, static_cast<int>(tf), button->palette, (button->state & State_Enabled),
                             button->text, QPalette::ButtonText);
            }
            return;
    }

    // For all other controls, draw the default
    super::drawControl(element, opt, p, widget);
}
