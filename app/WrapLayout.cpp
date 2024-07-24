

#include "WrapLayout.h"

WrapLayout::WrapLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

WrapLayout::WrapLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

WrapLayout::~WrapLayout() {
    QLayoutItem *item;
    while ((item = takeAt(0)) != nullptr) {
        delete item;
    }
}



void WrapLayout::addItem(QLayoutItem *item) {
    itemList.append(item);
}

int WrapLayout::horizontalSpacing() const {
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int WrapLayout::verticalSpacing() const {
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int WrapLayout::count() const {
    return itemList.size();
}

QLayoutItem *WrapLayout::itemAt(int index) const {
    return itemList.value(index);
}

QLayoutItem *WrapLayout::takeAt(int index) {
    if (index >= 0 && index < itemList.size()) {
        return itemList.takeAt(index);
    }
    return nullptr;
}

Qt::Orientations WrapLayout::expandingDirections() const {
    return 0;
}

bool WrapLayout::hasHeightForWidth() const {
    return true;
}

int WrapLayout::heightForWidth(int width) const {
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize WrapLayout::minimumSize() const {
    QSize size;
    for (QLayoutItem *item : itemList) {
        size = size.expandedTo(item->minimumSize());
    }
    size += QSize(2 * margin(), 2 * margin());
    return size;
}

void WrapLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize WrapLayout::sizeHint() const {
    return minimumSize();
}

int WrapLayout::doLayout(const QRect &rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : itemList) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1) spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1) spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly) item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int WrapLayout::smartSpacing(QStyle::PixelMetric pm) const {
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}
