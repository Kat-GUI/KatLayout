#ifndef KAT_LAYOUT
#define KAT_LAYOUT
#include<functional>
#include<list>
#include<typeindex>
#include<memory>
#include<string>
#include<limits>
#include<vector>
#include<math.h>
#include<queue>
#include<iostream>
#include<map>
#include<algorithm>
#include<memory>
#include<assert.h>
//#include"../cpp_DOM/src/DOM.h"
#include<windows.h>
namespace kat {
    HDC testHdc;
    class Limit {
    public:
        static const int Min = 0;
        static const int Max = std::numeric_limits<int>::max();

        Limit() : Limit(Min, Max) {}

        Limit(int min, int max) : max(max), min(min) {}

        static const Limit none;
        int min, max;

        int get(int val) {
            if (max != Limit::Max)val = std::min(val, max);
            if (min != Limit::Min)val = std::max(val, min);
            return val;
        }
    };

    struct Region {
        int l, t, w, h, r, b;
    };

    class Layout {
    public:
        Limit heightLimit, widthLimit;

        virtual void calcuRegion(Region anchor) = 0;

        virtual int getBoxMinWidth() = 0;

        virtual int getBoxMaxWidth() = 0;

        virtual int getBoxMinHeight() = 0;

        virtual int getBoxMaxHeight() = 0;

        virtual bool extendableInWidth() = 0;

        virtual bool extendableInHeight() = 0;

        virtual int getBoxWidth() = 0;

        virtual int getBoxHeight() = 0;

        HBRUSH brush;

        Layout() {
            brush = CreateSolidBrush(RGB(std::rand() % 200, std::rand() % 200, std::rand() % 200));
        }

        static bool inColumn;
        static bool inRow;
    protected:
        Region region;
        std::shared_ptr<Layout> child;
    };

    using LayoutPtr = std::shared_ptr<Layout>;

    class AppendableLayout : public Layout {
    protected:
        std::list<std::shared_ptr<Layout>> childs;
    public:
        int empty() { return childs.empty(); }

        virtual AppendableLayout &addChild(LayoutPtr layout) = 0;

        virtual AppendableLayout &removeChild(LayoutPtr layout) {
            childs.remove(layout);
        };
    };

    bool Layout::inColumn = false;
    bool Layout::inRow = false;

    LayoutPtr constraint(Limit width, Limit height, Layout *layout) {
        LayoutPtr tmp(layout);
        tmp->widthLimit = width;
        tmp->heightLimit = height;
        return tmp;
    }

    LayoutPtr constraint(Limit width, Limit height, LayoutPtr layout) {
        layout->widthLimit = width;
        layout->heightLimit = height;
        return layout;
    }

    void draw(Region region) {
        RECT r;
        r.left = region.l;
        r.top = region.t;
        r.right = region.r;
        r.bottom = region.b;
        FrameRect(testHdc, &r, CreateSolidBrush(RGB(0, 0, 0)));
    }

    const Limit Limit::none;

    class Margin : public Layout {
        int l, t, r, b;
    public:
        Margin(int left, int top, int right, int bottom) {
            l = left;
            t = top;
            r = right;
            b = bottom;
        }

        Margin(int px) : Margin(px, px, px, px) {}

        void setChild(Layout *layout) { child.reset(layout); }

        void setChild(LayoutPtr layout) { child = layout; }

        virtual void calcuRegion(Region anchor) override {
            region.l = anchor.l + l;
            region.t = anchor.t + t;
            region.r = anchor.r - r;
            region.b = anchor.b - b;
            region.w = region.r - region.l;
            region.h = region.b - region.t;
            int adjustW = (region.w - widthLimit.get(region.w)) / 2;
            int adjustH = (region.h - heightLimit.get(region.h)) / 2;
            region.l += adjustW;
            region.r -= adjustW;
            region.t += adjustH;
            region.b -= adjustH;
            region.w = region.r - region.l;
            region.h = region.b - region.t;
            draw(region);
            if (child)child->calcuRegion(region);
        }

        virtual int getBoxMinWidth() override { return l + widthLimit.min + r; }

        virtual int getBoxMaxWidth() override { return l + widthLimit.max + r; }

        virtual int getBoxMinHeight() override { return r + heightLimit.min + b; }

        virtual int getBoxMaxHeight() override { return t + heightLimit.max + b; }

        virtual bool extendableInWidth() override { return true; }

        virtual bool extendableInHeight() override { return true; }

        virtual int getBoxWidth() override { return l + region.w + r; };

        virtual int getBoxHeight() override { return t + region.h + b; }
    };

    enum class Horizontal {
        left, center, right
    };
    enum class Vertical {
        top, center, bottom
    };

    class Fixed : public Layout {
        int l, t, w, h, r, b;
        Horizontal xd;
        Vertical yd;
    public:
        static const int infinity;

        Fixed(Horizontal xDock, Vertical yDock, int left, int top, int width, int height, int right, int bottom)
                : xd(xDock), yd(yDock), l(left), t(top), w(width), h(height), r(right), b(bottom) {}

        Fixed(int left, int top, int width, int height)
                : Fixed(Horizontal::left, Vertical::top, left, top, width, height, 0, 0) {}

        Fixed(int width, int height)
                : Fixed(Horizontal::center, Vertical::center, 0, 0, width, height, 0, 0) {}

        void setChild(Layout *layout) { child.reset(layout); }

        virtual void calcuRegion(Region anchor) override {
            region.h = heightLimit.get(h == infinity ? anchor.h - t - b : h);
            region.w = widthLimit.get(w == infinity ? anchor.w - l - r : w);
            switch (xd) {
                case Horizontal::left:
                    region.l = anchor.l + l;
                    region.r = region.l + region.w;
                    break;
                case Horizontal::right:
                    region.r = anchor.r - r;
                    region.l = region.r - region.w;
                    break;
                case Horizontal::center:
                    region.l = anchor.l + (anchor.w - region.w) / 2;
                    region.r = anchor.r - (anchor.w - region.w) / 2;
            }
            switch (yd) {
                case Vertical::top:
                    region.t = anchor.t + t;
                    region.b = region.t + region.h;
                    break;
                case Vertical::bottom:
                    region.b = anchor.b - b;
                    region.t = region.b - region.h;
                    break;
                case Vertical::center:
                    region.t = anchor.t + (anchor.h - region.h) / 2;
                    region.b = anchor.b - (anchor.h - region.h) / 2;
            }
            if (child)child->calcuRegion(region);
            draw(region);
        }

        virtual int getBoxMinWidth() override { return l + region.w + r; }

        virtual int getBoxMaxWidth() override { return l + region.w + r; }

        virtual int getBoxMinHeight() override { return t + region.h + b; }

        virtual int getBoxMaxHeight() override { return t + region.h + b; }

        virtual bool extendableInWidth() override { return false; }

        virtual bool extendableInHeight() override { return false; }

        virtual int getBoxWidth() override { return l + region.w + r; };

        virtual int getBoxHeight() override { return t + region.h + b; }
    };

    const int Fixed::infinity = std::numeric_limits<int>::max();

//TODO 给以下四个类非扩展边添加自动贴合至父组件的功能
//TODO 测试Ratio
    class Ratio : public Layout {
        std::shared_ptr<Layout> child;
        float heightRatio, widthRatio;//divide height by width
    public:
        Ratio(float heightRatio, float widthRatio) : heightRatio(heightRatio), widthRatio(widthRatio) {}

        void setChild(Layout *layout) {
            child.reset(layout);
        }

        virtual void calcuRegion(Region anchor) override {
            int half;
            if (anchor.w > anchor.h) {
                region.h = anchor.h;
                region.w = (float) anchor.h / heightRatio * widthRatio;
                region.t = anchor.t;
                region.b = anchor.b;
                half = (anchor.w - region.w) / 2;
                region.l = anchor.l + half;
                region.r = region.l + region.w + half;
            } else {
                region.w = anchor.w;
                region.h = (float) anchor.w / widthRatio * heightRatio;
                region.l = anchor.l;
                region.r = anchor.r;
                half = (anchor.h - region.h) / 2;
                region.t = anchor.t + half;
                region.b = anchor.b + half;
            }
            draw(region);
            child->calcuRegion(region);
        }

        virtual int getBoxMinWidth() override { return 0; }

        virtual int getBoxMaxWidth() override { return 0; }

        virtual int getBoxMinHeight() override { return 0; }

        virtual int getBoxMaxHeight() override { return 0; }

        virtual bool extendableInWidth() override { return true; }

        virtual bool extendableInHeight() override { return true; }

        virtual int getBoxWidth() override { return region.w; };

        virtual int getBoxHeight() override { return region.h; }
    };

    class Dynamic : public Layout {
        std::list<std::shared_ptr<Layout>> childs;
        int minH = std::numeric_limits<int>::max(), minW = std::numeric_limits<int>::max(), maxH, maxW;
    public:
        Dynamic &addChild(Layout *layout) {
            minH = std::min(minH, layout->getBoxMaxHeight());
            minW = std::min(minW, layout->getBoxMinWidth());
            maxH = std::max(maxH, layout->getBoxMaxHeight());
            maxW = std::max(maxW, layout->getBoxMaxWidth());
            std::shared_ptr<Layout> newval;
            newval.reset(layout);
            childs.push_back(newval);
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            region = anchor;
            std::map<int, std::shared_ptr<Layout>> rank;

            if (Layout::inColumn)
                for (auto c:childs)
                    rank.insert(std::make_pair(anchor.w - c->getBoxMinWidth(), c));
            else if (Layout::inRow)
                for (auto c:childs)
                    rank.insert(std::make_pair(anchor.h - c->getBoxMinHeight(), c));
            else
                for (auto c:childs)
                    rank.insert(std::make_pair(anchor.w - c->getBoxMinWidth() + anchor.h - c->getBoxMinHeight(), c));

            for (auto r:rank) {
                child = r.second;
                if (r.first > -1 && r.second->getBoxMinWidth() <= anchor.w && r.second->getBoxMinHeight() <= anchor.h) {
                    break;
                }
            }
            if (Layout::inColumn && !child->extendableInWidth()) {
                region.w = child->getBoxWidth();
                region.r = region.l + region.w;
            }
            if (Layout::inRow && !child->extendableInHeight()) {
                region.h = child->getBoxHeight();
                region.b = region.t + region.h;
            }

            draw(region);
            if (child)child->calcuRegion(region);

        }

        virtual int getBoxMinWidth() override { return minW; }

        virtual int getBoxMaxWidth() override { return maxW; }

        virtual int getBoxMinHeight() override { return minH; }

        virtual int getBoxMaxHeight() override { return maxH; }

        virtual bool extendableInWidth() override { return true; }

        virtual bool extendableInHeight() override { return true; }

        virtual int getBoxWidth() override { return (bool) child ? child->getBoxWidth() : minW; };

        virtual int getBoxHeight() override { return (bool) child ? child->getBoxHeight() : minH; }
    };

    class Row : public AppendableLayout {
        int h = 0;
        std::map<Layout *, bool> squeezeFlag;
    public:
        std::function<void(std::shared_ptr<Layout>)> squeezeOut;
        std::function<void(std::shared_ptr<Layout>)> squeezeBack;

        Row(int height) : h(height) {}

        Row &addChild(Layout *layout) {
            std::shared_ptr<Layout> tmp;
            tmp.reset(layout);
            childs.push_back(tmp);
            return *this;
        }

        virtual Row &addChild(LayoutPtr layout) override {
            childs.push_back(layout);
            squeezeFlag[layout.get()] = false;
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            Layout::inColumn = true;
            std::vector<float> extWidth;
            float extSum = 0, restSpace = anchor.w, floating = 0;
            for (auto c:childs) {
                restSpace -= c->getBoxMinWidth();
                extWidth.push_back(c->getBoxMaxWidth() - c->getBoxMinWidth());
                extSum += extWidth.back();
            }
            region.l = anchor.l;
            region.t = anchor.t;
            region.h = h;
            region.b = region.t + region.h;
            int i = 0;
            for (auto iter = childs.begin(); iter != childs.end(); i++, iter++) {
                //squeeze
                if (region.l + (*iter)->getBoxMinWidth() >= anchor.r && squeezeOut) {
                    if (!squeezeFlag[iter->get()]) {
                        squeezeFlag[iter->get()] = true;
                        squeezeOut(*iter);
                    }
                    continue;
                } else if (squeezeBack && squeezeFlag[iter->get()]) {
                    squeezeFlag[iter->get()] = false;
                    squeezeBack(*iter);
                }

                int ext = restSpace * (extWidth[i]) / extSum;
                if (ext < 0)ext = 0;
                region.w = std::min((*iter)->getBoxMinWidth() + ext, (*iter)->getBoxMaxWidth());
                region.r = region.l + region.w;
                (*iter)->calcuRegion(region);
                int realW = (*iter)->getBoxWidth();
                region.l += realW;
                floating += realW;
                if (region.w > realW) {
                    extSum -= extWidth[i];
                    restSpace -= (realW - (*iter)->getBoxMinWidth());
                }
            }
            region.l = anchor.l;
            region.w = floating;
            region.r = region.l + region.w;
            Layout::inColumn = false;
            draw(region);
        }

        virtual int getBoxMinWidth() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxMinWidth();
            return ans;
        }

        virtual int getBoxMaxWidth() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxMaxWidth();
            return ans;
        }

        virtual int getBoxMinHeight() override { return h; }

        virtual int getBoxMaxHeight() override { return h; }

        virtual bool extendableInWidth() override { return true; }

        virtual bool extendableInHeight() override { return false; }

        virtual int getBoxWidth() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxWidth();
            return ans;
        };

        virtual int getBoxHeight() override { return region.h; }
    };

    class ExtendRow : public AppendableLayout {
        int h = 0;
        std::map<Layout *, bool> squeezeFlag;
    public:
        std::function<void(std::shared_ptr<Layout>)> squeezeOut;
        std::function<void(std::shared_ptr<Layout>)> squeezeBack;

        ExtendRow(int height) : h(height) {}

        ExtendRow &addChild(Layout *layout) {
            child.reset(layout);
            return addChild(child);
        }

        ExtendRow &addChild(std::shared_ptr<Layout> layout) {
            childs.push_back(layout);
            squeezeFlag[layout.get()] = false;
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            Layout::inColumn = true;
            int floating = 0;
            region.l = anchor.l;
            region.t = anchor.t;
            region.h = h;
            region.b = region.t + h;
            for (auto c:childs) {
                //squeeze
                if (region.l + c->getBoxMinWidth() >= anchor.r && squeezeOut) {
                    if (!squeezeFlag[c.get()]) {
                        squeezeFlag[c.get()] = true;
                        squeezeOut(c);
                    }
                    continue;
                } else if (squeezeBack && squeezeFlag[c.get()]) {
                    squeezeFlag[c.get()] = false;
                    squeezeBack(c);
                }
                region.w = c->getBoxMaxWidth();
                floating += region.w;
                region.r = region.l + region.w;
                c->calcuRegion(region);
                region.l += region.w;
            }
            region.l = anchor.l;
            region.w = floating;
            region.r = region.l + region.w;
            draw(region);
            Layout::inRow = false;
        }

        virtual int getBoxMinWidth() override { return getBoxMaxWidth(); }

        virtual int getBoxMaxWidth() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxMaxWidth();
            return ans;
        }

        virtual int getBoxMinHeight() override { return h; }

        virtual int getBoxMaxHeight() override { return h; }

        virtual bool extendableInWidth() override { return false; }

        virtual bool extendableInHeight() override { return false; }

        virtual int getBoxWidth() override { return getBoxMaxWidth(); };

        virtual int getBoxHeight() override { return region.h; }
    };

//TODO 测试Column
    class Column : public AppendableLayout {
        int w = 0;
        std::map<Layout *, bool> squeezeFlag;
    public:
        std::function<void(std::shared_ptr<Layout>)> squeezeOut;
        std::function<void(std::shared_ptr<Layout>)> squeezeBack;

        Column(int width) : w(width) {}

        Column &addChild(Layout *layout) {
            child.reset(layout);
            return addChild(child);
        }

        virtual Column &addChild(LayoutPtr layout) {
            childs.push_back(layout);
            squeezeFlag[layout.get()] = false;
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            Layout::inRow = true;
            std::vector<float> extHeight;
            float extSum = 0, restSpace = anchor.w, floating = 0;
            for (auto c:childs) {
                restSpace -= c->getBoxMinHeight();
                extHeight.push_back(c->getBoxMaxHeight() - c->getBoxMinHeight());
                extSum += extHeight.back();
            }
            region.l = anchor.l;
            region.t = anchor.t;
            region.w = w;
            region.r = region.l + region.w;
            int i = 0;
            for (auto iter = childs.begin(); iter != childs.end(); i++, iter++) {
                //squeeze
                if (region.l + (*iter)->getBoxMinWidth() >= anchor.r && squeezeOut) {
                    if (!squeezeFlag[iter->get()]) {
                        squeezeFlag[iter->get()] = true;
                        squeezeOut(*iter);
                    }
                    continue;
                } else if (squeezeBack && squeezeFlag[iter->get()]) {
                    squeezeFlag[iter->get()] = false;
                    squeezeBack(*iter);
                }
                int ext = restSpace * extHeight[i] / extSum;
                if (ext < 0)ext = 0;
                region.h = std::min((*iter)->getBoxMinHeight() + ext, (*iter)->getBoxMaxHeight());
                region.b = region.t + region.h;
                (*iter)->calcuRegion(region);
                int realH = (*iter)->getBoxHeight();
                region.t += realH;
                floating += realH;
                if (region.h > realH) {
                    extSum -= extHeight[i];
                    restSpace -= (realH - (*iter)->getBoxMinHeight());
                }
            }
            region.t = anchor.t;
            region.h = floating;
            region.b = region.t + region.h;
            Layout::inRow = false;
            draw(region);
        }

        virtual int getBoxMinHeight() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxMinHeight();
            return ans;
        }

        virtual int getBoxMaxHeight() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxMaxHeight();
            return ans;
        }

        virtual int getBoxMinWidth() override { return w; }

        virtual int getBoxMaxWidth() override { return w; }

        virtual bool extendableInWidth() override { return false; }

        virtual bool extendableInHeight() override { return true; }

        virtual int getBoxHeight() override {
            int ans = 0;
            for (auto c:childs) ans += c->getBoxHeight();
            return ans;
        };

        virtual int getBoxWidth() override { return region.w; }
    };

//TODO 测试ExtendColumn
    class ExtendColumn : public AppendableLayout {
        int w = 0;
        std::map<Layout *, bool> squeezeFlag;
    public:
        std::function<void(std::shared_ptr<Layout>)> squeezeOut;
        std::function<void(std::shared_ptr<Layout>)> squeezeBack;

        ExtendColumn(int width) : w(width) {}

        ExtendColumn &addChild(Layout *layout) {
            child.reset(layout);
            return addChild(child);
        }

        ExtendColumn &addChild(std::shared_ptr<Layout> layout) {
            childs.push_back(layout);
            squeezeFlag[layout.get()] = false;
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            Layout::inColumn = true;
            int floating = 0;
            region.l = anchor.l;
            region.t = anchor.t;
            region.w = w;
            region.r = region.l + w;
            for (auto c:childs) {
                //squeeze
                if (region.l + c->getBoxMinWidth() >= anchor.r && squeezeOut) {
                    if (!squeezeFlag[c.get()]) {
                        squeezeFlag[c.get()] = true;
                        squeezeOut(c);
                    }
                    continue;
                } else if (squeezeBack && squeezeFlag[c.get()]) {
                    squeezeFlag[c.get()] = false;
                    squeezeBack(c);
                }
                region.h = c->getBoxMaxHeight();
                floating += region.h;
                region.b = region.t + region.h;
                c->calcuRegion(region);
                region.t += region.h;
            }
            region.l = anchor.l;
            region.h = floating;
            region.b = region.t + region.h;
            draw(region);
            Layout::inRow = false;
        }

        virtual int getBoxMinWidth() override { return w; }

        virtual int getBoxMaxWidth() override { return w; }

        virtual int getBoxMinHeight() override { return getBoxMaxHeight(); }

        virtual int getBoxMaxHeight() override {
            int ans = 0;
            for (auto c:childs)ans += c->getBoxMaxHeight();
            return ans;
        }

        virtual bool extendableInWidth() override { return false; }

        virtual bool extendableInHeight() override { return false; }

        virtual int getBoxWidth() override { return w; };

        virtual int getBoxHeight() override { return getBoxMaxHeight(); }
    };

    enum class Direction {
        Horizontal, Vertical
    };

    class WarpPanel : public AppendableLayout {
        int minW = std::numeric_limits<int>::max(), sumW = 0, minH = std::numeric_limits<int>::max(), sumH = 0;
        Direction direction;
//    void calcuBound(){
//
//    }
    public:
        WarpPanel(Direction floating = Direction::Horizontal) : direction(floating) {}

        WarpPanel &addChild(Layout *layout) {
            child.reset(layout);
            return addChild(child);
        }

        WarpPanel &addChild(std::shared_ptr<Layout> layout) {
            childs.push_back(layout);
            int w = layout->getBoxMinWidth();
            int h = layout->getBoxMaxHeight();
            minW = std::min(minW, w);
            minH = std::min(minH, h);
            sumW += w;
            sumH += h;
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            Region subAnchor;
            int maxStacking = 0;
            if (direction == Direction::Horizontal) {
                subAnchor.l = anchor.l;
                subAnchor.t = anchor.t;
                for (auto iter = childs.begin(); iter != childs.end(); iter++) {
                    subAnchor.w = (*iter)->getBoxMinWidth();
                    subAnchor.h = (*iter)->getBoxMinHeight();
                    if (subAnchor.l + subAnchor.w > anchor.r) {
                        subAnchor.t += maxStacking;
                        maxStacking = 0;
                        subAnchor.l = anchor.l;
                    }
                    maxStacking = std::max(maxStacking, subAnchor.h);
                    subAnchor.r = subAnchor.l + subAnchor.w;
                    subAnchor.b = subAnchor.t + subAnchor.h;
                    (*iter)->calcuRegion(subAnchor);
                    subAnchor.l += subAnchor.w;
                }
            } else {
                subAnchor.l = anchor.l;
                subAnchor.t = anchor.t;
                for (auto iter = childs.begin(); iter != childs.end(); iter++) {
                    subAnchor.w = (*iter)->getBoxMinWidth();
                    subAnchor.h = (*iter)->getBoxMinHeight();
                    if (subAnchor.t + subAnchor.h > anchor.b) {
                        subAnchor.l += maxStacking;
                        maxStacking = 0;
                        subAnchor.t = anchor.t;
                    }
                    maxStacking = std::max(maxStacking, subAnchor.w);
                    subAnchor.r = subAnchor.l + subAnchor.w;
                    subAnchor.b = subAnchor.t + subAnchor.h;
                    (*iter)->calcuRegion(subAnchor);
                    subAnchor.t += subAnchor.h;
                }
            }
        }

        virtual int getBoxMinWidth() override { return minW; }

        virtual int getBoxMaxWidth() override { return sumW; }

        virtual int getBoxMinHeight() override { return minH; }

        virtual int getBoxMaxHeight() override { return sumH; }

        virtual bool extendableInWidth() override { return true; }

        virtual bool extendableInHeight() override { return false; }

        virtual int getBoxWidth() override { return region.w; };

        virtual int getBoxHeight() override { return region.h; }
    };

    class Stack : public AppendableLayout {
        std::list<std::shared_ptr<Layout>> childs;
    public:
        Stack() {}

        Stack &addChild(Layout *layout) {
            std::shared_ptr<Layout> child;
            child.reset(layout);
            return addChild(child);
        }

        Stack &addChild(std::shared_ptr<Layout> layout) {
            childs.push_back(child);
            return *this;
        }

        virtual void calcuRegion(Region anchor) override {
            region = anchor;
            for (auto c:childs)c->calcuRegion(anchor);
            draw(region);
        }

        virtual int getBoxMinWidth() override {
            int ans = std::numeric_limits<int>::max();
            for (auto c:childs)ans = std::min(ans, c->getBoxMinWidth());
            return ans;
        }

        virtual int getBoxMaxWidth() override {
            int ans = std::numeric_limits<int>::min();
            for (auto c:childs)ans = std::max(ans, c->getBoxMaxWidth());
            return ans;
        }

        virtual int getBoxMinHeight() override {
            int ans = std::numeric_limits<int>::max();
            for (auto c:childs)ans = std::min(ans, c->getBoxMinHeight());
            return ans;
        }

        virtual int getBoxMaxHeight() override {
            int ans = std::numeric_limits<int>::min();
            for (auto c:childs)ans = std::min(ans, c->getBoxMinHeight());
            return ans;
        }

        virtual bool extendableInWidth() override { return true; }

        virtual bool extendableInHeight() override { return true; }

        virtual int getBoxWidth() override { return region.w; };

        virtual int getBoxHeight() override { return region.h; }
    };

//TODO 增加ExtendGrid
    class Grid : public Layout {
    protected:
        struct Container {
            int x, y, spanX, spanY;
            std::shared_ptr<Layout> layout;
        };
        enum divideMode {
            count, scale, unit
        } mode = count;
        std::vector<float> xUnit, yUnit;
        int rowCount, colCount;
        std::vector<Container> content;
        std::vector<std::vector<int>> table;

        bool rangeHit(const int &hit, const int &lbound, const int &ubound) {
            return hit >= lbound && hit <= ubound;
        }

    public:
        Grid(int rowCount, int colCount) : colCount(colCount), rowCount(rowCount) {
            table.resize(rowCount);
            for (std::vector<int> &line:table)line.resize(colCount, -1);
        }

        Grid(std::initializer_list<float> rows, std::initializer_list<float> columns)
                : Grid(rows.size(), columns.size()) {
            xUnit.push_back(0);
            yUnit.push_back(0);
            for (float factor:columns)xUnit.push_back(factor + xUnit.back());
            for (float factor:rows) yUnit.push_back(factor + yUnit.back());
            mode = scale;
        }

        Grid(std::initializer_list<int> rows, std::initializer_list<int> columns)
                : Grid(rows.size(), columns.size()) {
            xUnit.push_back(0);
            yUnit.push_back(0);
            for (int r:rows) xUnit.push_back(r + xUnit.back());
            for (int c:columns) yUnit.push_back(c + yUnit.back());
            mode = unit;
        }

        Grid &addChild(int row, int col, int spanRow, int spanCol, Layout *layout) {
            if (!(rangeHit(row, 0, rowCount) && rangeHit(col, 0, colCount) &&
                  rangeHit(row + spanRow, 0, rowCount) && rangeHit(col + spanCol, 0, colCount)))
                throw "目标行列超出可用范围";
            Container container;
            container.layout.reset(layout);
            container.x = col;
            container.y = row;
            container.spanX = spanCol;
            container.spanY = spanRow;
            content.push_back(container);
            for (int y = row; y < row + spanRow; y++) {
                for (int x = col; x < col + spanCol; x++) {
                    if (table[y][x] != -1 && content[table[y][x]].layout)content[table[y][x]].layout.reset();
                    table[y][x] = content.size() - 1;
                }
            }

        }

        Grid &addChild(int row, int col, Layout *layout) {
            return addChild(row, col, 1, 1, layout);
        }

        virtual void calcuRegion(Region anchor) override {
            Region subAnchor;
            if (mode == count) {
                int unitX = anchor.w / colCount;
                int unitY = anchor.h / rowCount;
                for (auto container:content) {
                    if (!container.layout)continue;
                    subAnchor.l = anchor.l + unitX * container.x;
                    subAnchor.t = anchor.l + unitY * container.y;
                    subAnchor.w = unitX * container.spanX;
                    subAnchor.h = unitY * container.spanY;
                    subAnchor.r = subAnchor.l + subAnchor.w;
                    subAnchor.b = subAnchor.t + subAnchor.h;
                    container.layout->calcuRegion(subAnchor);
                }
                region = anchor;
            } else if (mode == scale) {
                for (auto container:content) {
                    if (!container.layout)continue;
                    subAnchor.l = anchor.l + anchor.w * xUnit[container.x];
                    subAnchor.t = anchor.t + anchor.h * yUnit[container.y];
                    subAnchor.r = anchor.l + anchor.w * xUnit[container.x + container.spanX];
                    subAnchor.b = anchor.t + anchor.h * yUnit[container.y + container.spanY];
                    subAnchor.w = subAnchor.r - subAnchor.l;
                    subAnchor.h = subAnchor.b - subAnchor.t;
                    container.layout->calcuRegion(subAnchor);
                }
                region = anchor;
            } else { //TODO 测试Grid unitMode
                for (auto container:content) {
                    if (!container.layout)continue;
                    subAnchor.l = anchor.l + xUnit[container.x];
                    subAnchor.t = anchor.t + yUnit[container.y];
                    subAnchor.r = anchor.l + xUnit[container.x + 1];
                    subAnchor.b = anchor.t + yUnit[container.y + 1];
                    subAnchor.w = subAnchor.r - subAnchor.l;
                    subAnchor.h = subAnchor.b - subAnchor.t;
                    container.layout->calcuRegion(subAnchor);
                }
                region.l = anchor.l;
                region.t = anchor.t;
                region.w = xUnit.back();
                region.h = yUnit.back();
                region.r = region.l + region.w;
                region.b = region.t + region.h;
            }
            draw(region);
        }

        //TODO 测试这几个接口
        virtual int getBoxMinWidth() override {
            if (mode == unit)return xUnit.back();
            int ans = 0;
            Layout *prvLayout;
            for (std::vector<int> line:table) {
                int lineWidth = 0;
                for (int index:line) {
                    if (prvLayout == content[index].layout.get())continue;
                    prvLayout = content[index].layout.get();
                    lineWidth += content[index].layout->getBoxMinWidth();
                }
                ans = std::max(ans, lineWidth);
            }
            return ans;
        }

        virtual int getBoxMaxWidth() override {
            if (mode == unit)return xUnit.back();
            int ans = 0;
            Layout *prvLayout;
            for (std::vector<int> line:table) {
                int lineWidth = 0;
                for (int index:line) {
                    if (prvLayout == content[index].layout.get())continue;
                    prvLayout = content[index].layout.get();
                    lineWidth += content[index].layout->getBoxMaxWidth();
                }
                ans = std::max(ans, lineWidth);
            }
            return ans;
        }

        virtual int getBoxMinHeight() override {
            if (mode == unit)return yUnit.back();
            int ans = 0;
            Layout *prvLayout;
            for (int x = 0; x < colCount; x++) {
                int colHeight = 0;
                for (int y = 0; y < rowCount; y++) {
                    int index = table[y][x];
                    if (prvLayout == content[index].layout.get())continue;
                    prvLayout = content[index].layout.get();
                    colHeight += content[index].layout->getBoxMinHeight();
                }
                ans = std::max(ans, colHeight);
            }
            return ans;
        }

        virtual int getBoxMaxHeight() override {
            if (mode == unit)return yUnit.back();
            int ans = 0;
            Layout *prvLayout;
            for (int x = 0; x < colCount; x++) {
                int colHeight = 0;
                for (int y = 0; y < rowCount; y++) {
                    int index = table[y][x];
                    if (prvLayout == content[index].layout.get())continue;
                    prvLayout = content[index].layout.get();
                    colHeight += content[index].layout->getBoxMaxHeight();
                }
                ans = std::max(ans, colHeight);
            }
            return ans;
        }

        virtual bool extendableInWidth() override { return mode != unit; }

        virtual bool extendableInHeight() override { return mode != unit; }

        virtual int getBoxWidth() override { return region.h; };

        virtual int getBoxHeight() override { return region.w; }
    };
}
#undef ASSERT
#endif
