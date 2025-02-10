
#ifndef WRAPLAYOUT_H
#define WRAPLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QWidgetItem>
#include <QStyle>
#include <QWidget>


#include <QLayout>
#include <QWidget>
#include <QStyle>
#include <QRect>
#include <QLayoutItem>
#include <QWidgetItem>

class WrapLayout : public QLayout {
public:
    explicit WrapLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit WrapLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~WrapLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    int m_hSpace;
    int m_vSpace;
};


#endif
