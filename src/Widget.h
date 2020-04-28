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
HDC testHdc;


class Layout{
protected:
    std::shared_ptr<Layout> child;
public:
    struct Region{int l,t,w,h,r,b;};
    virtual void calcuRegion(Region anchor)=0;
    virtual int getBoxMinWidth()=0;
    virtual int getBoxMaxWidth()=0;
    virtual int getBoxMinHeight()=0;
    virtual int getBoxMaxHeight()=0;
    virtual bool extendableInWidth()=0;
    virtual bool extendableInHeight()=0;
    virtual int getBoxWidth()=0;
    virtual int getBoxHeight()=0;
    HBRUSH brush;
    Layout(){
        brush = CreateSolidBrush(RGB(std::rand()%200,std::rand()%200,std::rand()%200));
    }
    static bool inColumn;
    static bool inRow;
protected:
    Region region;
};
bool Layout::inColumn=false;
bool Layout::inRow=false;
class Limit{
public:
    static const int Min=0;
    static const int Max=std::numeric_limits<int>::max();
    Limit():Limit(Min,Max){}
    Limit(int min,int max):max(max),min(min){}
    static const Limit none;
    int min,max;
    int get(int val){
        if(max!=Limit::Max)val = std::min(val,max);
        if(min!=Limit::Min)val = std::max(val,min);
        return val;
    }
};
void draw(Layout::Region region){
    RECT r;
    r.left=region.l;
    r.top=region.t;
    r.right=region.r;
    r.bottom=region.b;
    FrameRect(testHdc,&r,CreateSolidBrush(RGB(0,0,0)));
}
const Limit Limit::none;
class Margin:public Layout{
    int l,t,r,b;
    Limit w,h;
public:
    Margin(int left,int top,int right,int bottom,Limit width = Limit::none,Limit height = Limit::none){
        l=left;t=top;r=right;b=bottom;w=width;h=height;
    }
    Margin(int px,Limit width = Limit::none,Limit height = Limit::none):Margin(px,px,px,px,width,height){}
    void setChild(Layout* layout){child.reset(layout);}
    virtual void calcuRegion(Region anchor)override{
        region.l=anchor.l+l;
        region.t=anchor.t+t;
        region.r=anchor.r-r;
        region.b=anchor.b-b;
        region.w=region.r-region.l;
        region.h=region.b-region.t;
        int adjustW = (region.w - w.get(region.w))/2;
        int adjustH = (region.h - h.get(region.h))/2;
        region.l+=adjustW;
        region.r-=adjustW;
        region.t+=adjustH;
        region.b-=adjustH;
        region.w=region.r-region.l;
        region.h=region.b-region.t;
        draw(region);
        if(child)child->calcuRegion(region);
    }

    virtual int getBoxMinWidth()override{return l + w.min + r;}
    virtual int getBoxMaxWidth()override{return l + w.max + r;}
    virtual int getBoxMinHeight()override{return r + h.min + b;}
    virtual int getBoxMaxHeight()override{return t + h.max + b;}
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return true;}
    virtual int getBoxWidth()override{return l+region.w+r;};
    virtual int getBoxHeight()override{return t+region.h+b;}
};

enum class Horizontal{left,center,right};
enum class Vertical{top,center,bottom};

class Fixed:public Layout{
    int l,t,w,h,r,b;
    Horizontal xd;
    Vertical yd;
public:
    Fixed(Horizontal xDock,Vertical yDock,int left,int top,int width,int height,int right,int bottom)
        :xd(xDock),yd(yDock),l(left),t(top),w(width),h(height),r(right),b(bottom){}
    Fixed(int width,int height)
        :Fixed(Horizontal::center,Vertical::center,0,0,width,height,0,0){}
    void setChild(Layout* layout){child.reset(layout);}
    virtual void calcuRegion(Region anchor)override{
        region.h=h;
        region.w=w;
        switch(xd){
            case Horizontal::left:
                region.l=anchor.l+l;
                region.r=region.l+w;
                break;
            case Horizontal::right:
                region.r=anchor.r-r;
                region.l=region.r-w;
                break;
            case Horizontal::center:
                region.l=anchor.l + (anchor.w-w)/2;
                region.r=anchor.r - (anchor.w-w)/2;
        }
        switch(yd){
            case Vertical::top:
                region.t=anchor.t+t;
                region.b=region.t+h;
                break;
            case Vertical::bottom:
                region.b=anchor.b-b;
                region.t=region.b-h;
                break;
            case Vertical::center:
                region.t=anchor.t + (anchor.h-h)/2;
                region.b=anchor.b - (anchor.h-h)/2;
        }
        if(child)child->calcuRegion(region);
        draw(region);
    }
    virtual int getBoxMinWidth()override{return l + w + r;}
    virtual int getBoxMaxWidth()override{return l + w + r;}
    virtual int getBoxMinHeight()override{return t + h + b;}
    virtual int getBoxMaxHeight()override{return t + h + b;}
    virtual bool extendableInWidth()override{return false;}
    virtual bool extendableInHeight()override{return false;}
    virtual int getBoxWidth()override{return l+w+r;};
    virtual int getBoxHeight()override{return t+h+b;}
};

class Ratio:public Layout{
    std::shared_ptr<Layout> child;
    float heightRatio, widthRatio;//divide height by width
public:
    Ratio(float heightRatio,float widthRatio):heightRatio(heightRatio),widthRatio(widthRatio){}
    void setChild(Layout* layout){
        child.reset(layout);
    }
    virtual void calcuRegion(Region anchor)override {
        int half;
        if(anchor.w>anchor.h){
            region.h=anchor.h;
            region.w=(float)anchor.h/heightRatio * widthRatio;
            region.t=anchor.t;
            region.b=anchor.b;
            half = (anchor.w-region.w)/2;
            region.l=anchor.l+half;
            region.r=region.l+region.w+half;
        }
        else{
            region.w=anchor.w;
            region.h=(float)anchor.w/widthRatio * heightRatio;
            region.l=anchor.l;
            region.r=anchor.r;
            half = (anchor.h-region.h)/2;
            region.t=anchor.t+half;
            region.b=anchor.b+half;
        }
        draw(region);
        child->calcuRegion(region);
    }
    virtual int getBoxMinWidth()override{return 0;}
    virtual int getBoxMaxWidth()override{return 0;}
    virtual int getBoxMinHeight()override{return 0;}
    virtual int getBoxMaxHeight()override{return 0;}
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return true;}
    virtual int getBoxWidth()override{return region.w;};
    virtual int getBoxHeight()override{return region.h;}
};

class Dynamic:public Layout {
    std::list<std::shared_ptr<Layout>> childs;
    int minH=std::numeric_limits<int>::max(),minW=std::numeric_limits<int>::max(),maxH,maxW;
public:
    Dynamic &addChild(Layout *layout) {
        minH=std::min(minH,layout->getBoxMaxHeight());
        minW=std::min(minW,layout->getBoxMinWidth());
        maxH=std::max(maxH,layout->getBoxMaxHeight());
        maxW=std::max(maxW,layout->getBoxMaxWidth());
        std::shared_ptr<Layout> newval;
        newval.reset(layout);
        childs.push_back(newval);
        return *this;
    }
    virtual void calcuRegion(Region anchor) override {
        region=anchor;
        std::map<int,std::shared_ptr<Layout>> rank;

        if(Layout::inColumn)
            for (auto c:childs)
                rank.insert(std::make_pair(anchor.w - c->getBoxMinWidth(), c));
        else if(Layout::inRow)
            for (auto c:childs)
                rank.insert(std::make_pair(anchor.h - c->getBoxMinHeight(), c));
        else
            for (auto c:childs)
                rank.insert(std::make_pair(anchor.w - c->getBoxMinWidth()+anchor.h-c->getBoxMinHeight(), c));

        for (auto r:rank) {
            child = r.second;
            if (r.first > -1 && r.second->getBoxMinWidth() <= anchor.w && r.second->getBoxMinHeight() <= anchor.h) {
                break;
            }
        }
        if(Layout::inColumn && !child->extendableInWidth()) {
            region.w = child->getBoxWidth();
            region.r = region.l + region.w;
        }
        if(Layout::inRow && !child->extendableInHeight()) {
            region.h = child->getBoxHeight();
            region.b = region.t + region.h;
        }

        draw(region);
        if(child)child->calcuRegion(region);

    }
    virtual int getBoxMinWidth()override{return minW;}
    virtual int getBoxMaxWidth()override{return maxW;}
    virtual int getBoxMinHeight()override{return minH;}
    virtual int getBoxMaxHeight()override{return maxH;}
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return true;}
    virtual int getBoxWidth()override{return (bool)child ? child->getBoxWidth():minW;};
    virtual int getBoxHeight()override{return (bool)child ? child->getBoxHeight():minH;}
};

//TODO 增加"挤压出子组件到另一个组件中"的功能
class Column:public Layout{
    std::vector<std::shared_ptr<Layout>> childs;
    int h=0;
public:
    Column(int height):h(height){}
    Column& addChild(Layout* layout){
        std::shared_ptr<Layout> tmp;
        tmp.reset(layout);
        childs.push_back(tmp);
        return *this;
    }
    virtual void calcuRegion(Region anchor)override{
        Layout::inColumn=true;
        std::vector<float> extWidth;
        float extSum=0,restSpace=anchor.w,floating=0;
        for(auto c:childs){
            restSpace-=c->getBoxMinWidth();
            extWidth.push_back(c->getBoxMaxWidth()-c->getBoxMinWidth());
            extSum+=extWidth.back();
        }
        region.l=anchor.l;
        region.t=anchor.t;
        region.h=h;
        region.b=region.t+region.h;
        for(int i=0;i<childs.size();i++){
            int ext = restSpace*(extWidth[i])/extSum;
            if(ext<0)ext=0;
            region.w=std::min(childs[i]->getBoxMinWidth()+ext,childs[i]->getBoxMaxWidth());
            region.r=region.l+region.w;
            childs[i]->calcuRegion(region);
            int realW = childs[i]->getBoxWidth();
            region.l+=realW;
            floating+=realW;
            if(region.w>realW) {
                extSum -= extWidth[i];
                restSpace -= (realW - childs[i]->getBoxMinWidth());
            }
        }
        region.l=anchor.l;
        region.w=floating;
        region.r=region.l+region.w;
        Layout::inColumn=false;
        draw(region);
    }
    virtual int getBoxMinWidth()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxMinWidth();
        return ans;
    }
    virtual int getBoxMaxWidth()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxMaxWidth();
        return ans;
    }
    virtual int getBoxMinHeight()override{return h;}
    virtual int getBoxMaxHeight()override{return h;}
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return false;}
    virtual int getBoxWidth()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxWidth();
        return ans;
    };
    virtual int getBoxHeight()override{return region.h;}
};

class ExtendColumn:public Layout{
    std::vector<std::shared_ptr<Layout>> childs;
    int h=0;
public:
    ExtendColumn(int height):h(height){}
    ExtendColumn& addChild(Layout* layout){
        std::shared_ptr<Layout> tmp;
        tmp.reset(layout);
        childs.push_back(tmp);
        return *this;
    }
    virtual void calcuRegion(Region anchor)override{
        Layout::inColumn=true;
        int floating=0;
        region.l=anchor.l;
        region.t=anchor.t;
        region.h=h;
        region.b=region.t+h;
        for(auto c:childs){
            region.w=c->getBoxMaxWidth();
            floating+=region.w;
            region.r=region.l+region.w;
            c->calcuRegion(region);
            region.l+=region.w;
        }
        region.l=anchor.l;
        region.w=floating;
        region.r=region.l+region.w;
        draw(region);
        Layout::inRow=false;
    }
    virtual int getBoxMinWidth()override{return getBoxMaxWidth();}
    virtual int getBoxMaxWidth()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxMaxWidth();
        return ans;
    }
    virtual int getBoxMinHeight()override{return h;}
    virtual int getBoxMaxHeight()override{return h;}
    virtual bool extendableInWidth()override{return false;}
    virtual bool extendableInHeight()override{return false;}
    virtual int getBoxWidth()override{return getBoxMaxWidth();};
    virtual int getBoxHeight()override{return region.h;}
};

//TODO 增加"挤压出子组件到另一个组件中"的功能
//TODO 测试以下两个类
class Rows:public Layout{
    std::vector<std::shared_ptr<Layout>> childs;
    int w=0;
public:
    Rows(int width):w(width){}
    Rows& addChild(Layout* layout){
        std::shared_ptr<Layout> tmp;
        tmp.reset(layout);
        childs.push_back(tmp);
        return *this;
    }
    virtual void calcuRegion(Region anchor)override{
        Layout::inRow=true;
        std::vector<float> extHeight;
        float extSum=0,restSpace=anchor.w,floating=0;
        for(auto c:childs){
            restSpace-=c->getBoxMinHeight();
            extHeight.push_back(c->getBoxMaxHeight()-c->getBoxMinHeight());
            extSum+=extHeight.back();
        }
        region.l=anchor.l;
        region.t=anchor.t;
        region.w=w;
        region.r=region.l+region.w;
        for(int i=0;i<childs.size();i++){
            int ext = restSpace * extHeight[i]/extSum;
            if(ext<0)ext=0;
            region.h=std::min(childs[i]->getBoxMinHeight()+ext,childs[i]->getBoxMaxHeight());
            region.b=region.t+region.h;
            childs[i]->calcuRegion(region);
            int realH = childs[i]->getBoxHeight();
            region.t+=realH;
            floating+=realH;
            if(region.h>realH) {
                extSum -= extHeight[i];
                restSpace -= (realH - childs[i]->getBoxMinHeight());
            }
        }
        region.t=anchor.t;
        region.h=floating;
        region.b=region.t+region.h;
        Layout::inRow=false;
        draw(region);
    }
    virtual int getBoxMinHeight()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxMinHeight();
        return ans;
    }
    virtual int getBoxMaxHeight()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxMaxHeight();
        return ans;
    }
    virtual int getBoxMinWidth()override{return w;}
    virtual int getBoxMaxWidth()override{return w;}
    virtual bool extendableInWidth()override{return false;}
    virtual bool extendableInHeight()override{return true;}
    virtual int getBoxHeight()override{
        int ans=0;
        for(auto c:childs) ans+=c->getBoxHeight();
        return ans;
    };
    virtual int getBoxWidth()override{return region.w;}
};

class ExtendRow:public Layout{
    std::vector<std::shared_ptr<Layout>> childs;
    int w=0;
public:
    ExtendRow(int width):w(width){}
    ExtendRow& addChild(Layout* layout){
        std::shared_ptr<Layout> tmp;
        tmp.reset(layout);
        childs.push_back(tmp);
        return *this;
    }
    virtual void calcuRegion(Region anchor)override{
        Layout::inColumn=true;
        int floating=0;
        region.l=anchor.l;
        region.t=anchor.t;
        region.w=w;
        region.r=region.l+w;
        for(auto c:childs){
            region.h=c->getBoxMaxHeight();
            floating+=region.h;
            region.b=region.t+region.h;
            c->calcuRegion(region);
            region.t+=region.h;
        }
        region.l=anchor.l;
        region.h=floating;
        region.b=region.t+region.h;
        draw(region);
        Layout::inRow=false;
    }
    virtual int getBoxMinWidth()override{return w;}
    virtual int getBoxMaxWidth()override{return w;}
    virtual int getBoxMinHeight()override{ return getBoxMaxHeight(); }
    virtual int getBoxMaxHeight()override{
        int ans=0;
        for(auto c:childs)ans+=c->getBoxMaxHeight();
        return ans;
    }
    virtual bool extendableInWidth()override{return false;}
    virtual bool extendableInHeight()override{return false;}
    virtual int getBoxWidth()override{return w;};
    virtual int getBoxHeight()override{return getBoxMaxHeight();}
};

enum class Direction{Horizontal,Vertical};
class WarpPanel:public Layout{
    std::vector<std::shared_ptr<Layout>> childs;
    int minW = std::numeric_limits<int>::max(),sumW=0,minH=std::numeric_limits<int>::max(),sumH=0;
    Direction direction;
//    void calcuBound(){
//
//    }
public:
    WarpPanel(Direction floating=Direction::Horizontal):direction(floating){}
    WarpPanel& addChild(Layout* layout){
        std::shared_ptr<Layout> child;
        child.reset(layout);
        childs.push_back(child);
        int w = child->getBoxMinWidth();
        int h = child->getBoxMaxHeight();
        minW=std::min(minW,w);
        minH=std::min(minH,h);
        sumW+=w;
        sumH+=h;
        return *this;
    }
    virtual void calcuRegion(Region anchor)override{
        Region subAnchor;
        int maxStacking=0;
        if(direction==Direction::Horizontal){
            subAnchor.l=anchor.l;
            subAnchor.t=anchor.t;
            for(auto iter=childs.begin();iter!=childs.end();iter++){
                subAnchor.w=(*iter)->getBoxMinWidth();
                subAnchor.h=(*iter)->getBoxMinHeight();
                if(subAnchor.l+subAnchor.w > anchor.r){
                    subAnchor.t+=maxStacking;
                    maxStacking=0;
                    subAnchor.l=anchor.l;
                }
                maxStacking=std::max(maxStacking,subAnchor.h);
                subAnchor.r=subAnchor.l+subAnchor.w;
                subAnchor.b=subAnchor.t+subAnchor.h;
                (*iter)->calcuRegion(subAnchor);
                subAnchor.l += subAnchor.w;
            }
        }
        else{
            subAnchor.l=anchor.l;
            subAnchor.t=anchor.t;
            for(auto iter=childs.begin();iter!=childs.end();iter++){
                subAnchor.w=(*iter)->getBoxMinWidth();
                subAnchor.h=(*iter)->getBoxMinHeight();
                if(subAnchor.t+subAnchor.h > anchor.b){
                    subAnchor.l+=maxStacking;
                    maxStacking=0;
                    subAnchor.t=anchor.t;
                }
                maxStacking=std::max(maxStacking,subAnchor.w);
                subAnchor.r=subAnchor.l+subAnchor.w;
                subAnchor.b=subAnchor.t+subAnchor.h;
                (*iter)->calcuRegion(subAnchor);
                subAnchor.t += subAnchor.h;
            }
        }
    }
    virtual int getBoxMinWidth()override{return minW;}
    virtual int getBoxMaxWidth()override{return sumW;}
    virtual int getBoxMinHeight()override{return minH;}
    virtual int getBoxMaxHeight()override{return sumH;}
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return false;}
    virtual int getBoxWidth()override{return region.w;};
    virtual int getBoxHeight()override{return region.h;}
};

class Stack:public Layout{
    std::list<std::shared_ptr<Layout>> childs;
public:
    Stack(){}
    Stack& addChild(Layout* layout){
        std::shared_ptr<Layout> child;
        child.reset(layout);
        childs.push_back(child);
        return *this;
    }
    virtual void calcuRegion(Region anchor)override{
        region=anchor;
        for(auto c:childs)c->calcuRegion(anchor);
        draw(region);
    }
    virtual int getBoxMinWidth()override{
        int ans = std::numeric_limits<int>::max();
        for(auto c:childs)ans = std::min(ans,c->getBoxMinWidth());
        return ans;
    }
    virtual int getBoxMaxWidth()override{
        int ans = std::numeric_limits<int>::min();
        for(auto c:childs)ans = std::max(ans,c->getBoxMaxWidth());
        return ans;
    }
    virtual int getBoxMinHeight()override{
        int ans = std::numeric_limits<int>::max();
        for(auto c:childs)ans = std::min(ans,c->getBoxMinHeight());
        return ans;
    }
    virtual int getBoxMaxHeight()override{
        int ans = std::numeric_limits<int>::min();
        for(auto c:childs)ans = std::min(ans,c->getBoxMinHeight());
        return ans;
    }
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return true;}
    virtual int getBoxWidth()override{return region.w;};
    virtual int getBoxHeight()override{return region.h;}
};

//TODO 增加ExtendGrid
//TODO 增加Unit序列构造函数
class Grid:public Layout{
protected:
    struct Container{
        int x,y,spanX,spanY;
        std::shared_ptr<Layout> layout;
    };
    enum divideMode{count,scale}mode=count;
    std::vector<float> xScale,yScale;
    int rowCount,colCount;
    std::vector<Container> content;
    std::vector<std::vector<int>> table;
    bool rangeHit(const int& hit,const int& lbound,const int& ubound){
        return hit>=lbound && hit<=ubound;
    }
public:
    Grid(int rowCount,int colCount):colCount(colCount),rowCount(rowCount){
        table.resize(rowCount);
        for(std::vector<int>& line:table)line.resize(colCount,-1);
    }
    Grid(std::initializer_list<float> rows,std::initializer_list<float> columns)
        :Grid(rows.size(),rows.size()){
        xScale.push_back(0);
        float sum=0;
        for(float factor:columns){
            sum+=factor;
            xScale.push_back(sum);
        }
        sum=0;
        yScale.push_back(0);
        for(float factor:rows){
            sum+=factor;
            yScale.push_back(sum);
        }
        mode=scale;
    }
    Grid& addChild(int row,int col,int spanRow,int spanCol,Layout* layout){
        if(!(rangeHit(row,0,rowCount) && rangeHit(col,0,colCount) &&
        rangeHit(row+spanRow,0,rowCount) && rangeHit(col+spanCol,0,colCount)))
            throw "目标行列超出可用范围";
        Container container;
        container.layout.reset(layout);
        container.x=col;
        container.y=row;
        container.spanX=spanCol;
        container.spanY=spanRow;
        content.push_back(container);
        for(int y=row;y<row+spanRow;y++){
            for(int x=col;x<col+spanCol;x++){
                if(table[y][x]!=-1&&content[table[y][x]].layout)content[table[y][x]].layout.reset();
                table[y][x]=content.size()-1;
            }
        }

    }
    Grid& addChild(int row,int col,Layout* layout){
        return addChild(row,col,1,1,layout);
    }
    //TODO 完成这几个函数覆写
    virtual void calcuRegion(Region anchor)override{
        Region subAnchor;
        if(mode==count) {
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
        }
        else{
            for(auto container:content){
                if(!container.layout)continue;
                subAnchor.l = anchor.l + anchor.w * xScale[container.x];
                subAnchor.t = anchor.t + anchor.h * yScale[container.y];
                subAnchor.r = anchor.l + anchor.w * xScale[container.x+container.spanX];
                subAnchor.b = anchor.t + anchor.h * yScale[container.y+container.spanY];
                subAnchor.w = subAnchor.r - subAnchor.l;
                subAnchor.h = subAnchor.b - subAnchor.t;
                container.layout->calcuRegion(subAnchor);
            }
        }
        region=anchor;
    }
    virtual int getBoxMinWidth()override{return 0;}
    virtual int getBoxMaxWidth()override{return 0;}
    virtual int getBoxMinHeight()override{return 0;}
    virtual int getBoxMaxHeight()override{return 0;}
    virtual bool extendableInWidth()override{return true;}
    virtual bool extendableInHeight()override{return true;}
    virtual int getBoxWidth()override{return 0;};
    virtual int getBoxHeight()override{return 0;}
};
//
//class ExtendGrid:public Grid{
//public:
//    ExtendGrid(int rowCount,int colCount):Grid(rowCount,colCount){}
//    Grid& addChild(int row,int col,int spanRow,int spanCol,Layout* layout){
//        if(rangeHit(row,0,rowCount) && rangeHit(col,0,colCount) &&
//           rangeHit(row+spanRow,0,rowCount) && rangeHit(col+spanCol,0,colCount))
//            throw "目标行列超出可用范围";
//        Container container;
//        container.layout.reset(layout);
//        container.x=col;
//        container.y=row;
//        container.spanX=spanCol;
//        container.spanY=spanRow;
//        content.push_back(container);
//        for(int y=row;y<row+spanRow;y++){
//            for(int x=col;x<col+spanCol;x++){
//                if(content[table[y][x]].layout)content[table[y][x]].layout.reset();
//                table[y][x]=content.size()-1;
//            }
//        }
//
//    }
//    Grid& addChild(int row,int col,Layout* layout){
//        return addChild(row,col,1,1,layout);
//    }
//    virtual void calcuRegion(Region anchor)override{
//        Region subAnchor;
//        int unitX = anchor.w/colCount;
//        int unitY = anchor.h/rowCount;
//        for(auto container:content){
//            if(!container.layout)continue;
//            subAnchor.l = anchor.l + unitX * container.x;
//            subAnchor.t = anchor.l + unitY * container.y;
//            subAnchor.w = unitX * container.spanX;
//            subAnchor.h = unitY * container.spanY;
//            subAnchor.r = subAnchor.l+subAnchor.w;
//            subAnchor.b = subAnchor.t+subAnchor.h;
//            container.layout->calcuRegion(subAnchor);
//        }
//    }
//    virtual int getBoxMinWidth()override{return 0;}
//    virtual int getBoxMaxWidth()override{return 0;}
//    virtual int getBoxMinHeight()override{return 0;}
//    virtual int getBoxMaxHeight()override{return 0;}
//    virtual bool extendableInWidth()override{return true;}
//    virtual bool extendableInHeight()override{return true;}
//    virtual int getBoxWidth()override{return 0;};
//    virtual int getBoxHeight()override{return 0;}
//};

//class Layout{
//protected:
//    struct Axis{
//        int head=0,body=0,tail=0;
//        bool compressed = false;
//        struct Limit{
//            int max=std::numeric_limits<int>::max(),min=0;
//        }limit;
//    }x,y;
//    std::shared_ptr<Layout> child;
//public:
//    struct Region{int l,t,w,h,r,b;}region;
//    enum LayoutAlignMode{leftAndTop,leftAndBottom,rightAndTop,rightAndBottom}alignmode=leftAndTop;
//    int getMinWidth(){return x.limit.min;}
//    int getMaxWidth(){return x.limit.max;}
//    int getLeft(){ return x.head; }
//    int getRight(){ return x.tail; }
//    int getWidth(){
//        if(x.compressed) return (bool)child ? child->getBoxMinWidth() : 0;
//        else return x.body;
//    }
//    bool extendedableInWidth(){return x.body==0;}
//    int getBoxMinWidth(){return getLeft() + getRight() + extendedableInWidth() ? x.limit.min : x.body;}
//    int getBoxMaxWidth(){return getLeft() + getRight() + extendedableInWidth() ? x.limit.max : x.body;}
//
//    int getMinHeight(){return y.limit.min;}
//    int getMaxHeight(){return y.limit.max;}
//    int getTop(){ return y.head; }
//    int getBottom(){ return y.tail; }
//    int getHeight(){
//        if(y.compressed) return (bool)child ? child->getBoxMinHeight() : 0;
//        else  return y.body;
//    }
//    bool extendedableInHeight(){return y.body==0;}
//    int getBoxMinHeight(){return getTop() + getBottom() + extendedableInHeight() ? y.limit.min : y.body;}
//    int getBoxMaxHeight(){return getLeft() + getRight() + extendedableInHeight() ? y.limit.max : y.body;}
//
//    Layout& setMinWidth(int val){x.limit.min=val;return *this;}
//    Layout& setMaxWidth(int val){x.limit.max=val;return *this;}
//    Layout& setLeft(int val){ x.head = val;return *this;}
//    Layout& sestRight(int val){ x.tail = val;return *this;}
//    Layout& setWidth(int val){
//        val=std::max(x.limit.min,val);
//        val=std::min(x.limit.max,val);
//        if(x.compressed) throw "无法设置宽度"; else x.body = val;
//        return *this;
//    }
//
//    Layout& setMinHeight(int val){y.limit.min=val;return *this;}
//    Layout& setMaxHeight(int val){y.limit.max=val;return *this;}
//    Layout& setTop(int val){y.head=val;return *this;}
//    Layout& setBottom(int val){y.tail=val;return *this;}
//    Layout& setHeight(int val){
//        val=std::max(y.limit.min,val);
//        val=std::min(y.limit.max,val);
//        if(y.compressed) throw "无法设置宽度"; else y.body = val;
//        return *this;
//    }
//
//    virtual void calcuRegion(Region anchor){
//
//    }
//};


//
//class Node{
//    Node* parent=nullptr;
//    std::list<Node*> child;
//    static std::map<std::string,Node*> map;
//public:
//    std::string id;
//public:
//    int childCount(){
//        if(child.empty())return 0;
//        return child.size();
//    }
//    void add(Node* node){
//        child.push_back(node);
//        node->parent=this;
//        map.insert(std::make_pair(node->id,node));
//    }
//    void remove(Node* node){
//        child.remove(node);
//        node->parent=nullptr;
//        map.erase(map.find(node->id));
//    }
//    template<typename T>
//    void foreach(std::function<void(T&)> func){
//        for(auto iter=child.begin();iter!=child.end();iter++)
//            if(*iter!=nullptr) func(*dynamic_cast<T*>(*iter));
//    }
//    std::list<Node*>::iterator begin(){return child.begin();}
//    std::list<Node*>::iterator end(){return child.end();}
//    template<typename R=Node>
//    R* getParent(){return dynamic_cast<R*>(parent);}
//    template<typename R=Node>
//    R* getfirstChild(){
//        if(child.empty())return nullptr;
//        return dynamic_cast<R*>(*child.begin());
//    }
//    void setfirstChild(Node* node,bool autoDelete=true){
//        if(child.empty())
//            child.push_back(node);
//        else{
//            (*child.begin())=node;
//            if(autoDelete)
//                delete *child.begin();
//            else
//                if(node!=nullptr)(*child.begin())->parent=nullptr;
//        }
//        if(node!=nullptr)node->parent=this;
//    }
//    static Node* find(std::string id){
//        return map.find(id)->second;
//    }
//    virtual void processMessage(Enum msg){
//        //onMessage(msg);
//        for(auto node:child)
//            node->processMessage(msg);
//    };
////    virtual void onMessage(Enum msg)=0;
//};
//std::map<std::string,Node*> Node::map;
//
//class Widget;
//class Limited;
//class Layout:public Node
//{
//    friend Limited;
//public:
//    Layout(){
//        x.type=Axis::X;
//        y.type=Axis::Y;
//    }
//    struct Region{
//        const float obsolete=std::numeric_limits<int>::lowest();
//        float x,y,w,h,r,b;
//        void setObsolete(){
////            x=y=w=h=r=b=obsolete;
//        }
//    }region;
//protected:
//
//    static const float empty;
//	struct Axis{
//	    enum Enum{X,Y}type;
//	    float head=empty;
//	    float body=empty;
//	    float tail=empty;
//	    bool scaleHead=false,scaleBody=false,scaleTail=false;
//	    bool extended=false;//为真时，body也要设为非empty
//	    struct Limit{
//	        float max=empty;
//	        float min=empty;
//	    }limit;
//	}x,y;
//
//protected:
//    float limit(const Axis &axis,const float& distance){
//        float ans=distance;
//        if(axis.limit.max!=empty && distance > axis.limit.max)ans=axis.limit.max;
//        if(axis.limit.min!=empty && distance < axis.limit.min)ans=axis.limit.min;
//        return ans;
//    }
//public:
//    virtual bool expandable(Axis::Enum axis){ return true; }
//    //蛤？父类长宽由我定？
//    //返回最小值
//    virtual float getFillerLength(Axis::Enum type){
//        float ans = 0;
//        const Axis &axis = type==Axis::X ? x : y;
//        if(axis.extended){
//            //我也不知道啊
//            if(childCount()>0) ans = getfirstChild<Layout>()->getFillerLength(type);  //去我子类那问吧
//            else throw "无法计算坐标";                                        //没子类了
//        }
//        else{
//            //我知道
//            if(axis.limit.min!=empty) ans = axis.limit.min;
//            else if(axis.scaleBody||axis.scaleHead||axis.scaleTail||axis.body==empty) ans = 0;
//            else ans = limit(axis,axis.body);
//        }
//        return ans + (axis.head==empty ? 0:axis.head) + (axis.tail==empty ? 0:axis.tail);//加上缝隙
//    }
//protected:
//    void calcuAxis(Axis &axis,float &begin,float &distance,float &end,
//            const float& parent_begin,const float& parent_distance){
//        float tmp;
//        //直接穷举 通俗易懂
//        if(axis.head==empty && axis.body!=empty && axis.tail==empty){
//            //[   ],body,[   ]
//            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
//            distance = limit(axis, axis.extended ? getfirstChild<Layout>()->getFillerLength(axis.type) : tmp);
//            begin = (parent_distance - distance)/2;
//            end = begin;
//        }
//        else if(axis.head==empty && axis.body!=empty && axis.tail!=empty){
//            //[   ],body,tail
//            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
//            distance = limit(axis, axis.extended ? getfirstChild<Layout>()->getFillerLength(axis.type) : tmp);
//            end = axis.scaleTail ? axis.tail*parent_distance : axis.tail;
//            begin = parent_distance - distance - end;
//        }
//        else if(axis.head != empty && axis.body!=empty && axis.tail==empty){
//            //head,body,[   ]
//            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
//            distance = limit(axis, axis.extended ? getfirstChild<Layout>()->getFillerLength(axis.type) : tmp);
//            begin = axis.scaleHead ? axis.head*parent_distance : axis.head;
//            end = parent_distance - distance - begin;
//        }
//        else if(axis.head!=empty && axis.body==empty && axis.tail!=empty){
//            //head,[   ],tail
//            begin = axis.scaleHead ? axis.head*parent_distance : axis.head;
//            end = axis.scaleTail ? axis.tail*parent_distance : axis.tail;
//            tmp = parent_distance - begin - end;
//            distance = limit(axis, axis.extended ? getfirstChild<Layout>()->getFillerLength(axis.type) : tmp);
//            if(tmp!=distance) {
//                begin += (tmp - distance)/2;
//                end += (tmp - distance)/2;
//            }
//        }else{
//            //避免非parent::extended时执行此代码，否则会有意外的结果
//            begin=axis.head;
//            end = axis.tail;
//            distance=limit(axis, axis.extended ? getfirstChild<Layout>()->getFillerLength(axis.type) : axis.body);
//        }
//        begin+=parent_begin;
//        end+=parent_begin;
//        //TODO:安排整体缩放
//    }
//public:
//    virtual void calcuRegion(Layout* container){
//        //getfirstChild<Layout>()->region.setObsolete();
//        calcuAxis(x,region.x,region.w,region.r,container->region.x,container->region.w);
//        calcuAxis(y,region.y,region.h,region.b,container->region.y,container->region.h);
//	}
//
//	//TODO: 把这里set get都删掉
//	//下面到undef之前都是烦人的口水代码
//#define ASSERT static_assert(std::is_same<T,int>::value||std::is_same<T,float>::value||std::is_same<T,double>::value,"The template parameter must be int or float.")
//	template<typename T>
//	void setLeft(T int_or_float){
//        ASSERT;
//        x.scaleHead=std::is_same<T,float>::value;
//        x.head=int_or_float;
//    }
//    template<typename T>
//    void setTop(T int_or_float){
//        ASSERT;
//        y.scaleHead=std::is_same<T,float>::value;
//        y.head=int_or_float;
//    }
//    template<typename T>
//    void setRight(T int_or_float){
//        ASSERT;
//        x.scaleTail=std::is_same<T,float>::value;
//        x.tail=int_or_float;
//    }
//    template<typename T>
//    void setBottom(T int_or_float){
//        ASSERT;
//        y.scaleTail=std::is_same<T,float>::value;
//        y.tail=int_or_float;
//    }
//    template<typename T>
//    void setHeight(T int_or_float){
//        ASSERT;
//        y.scaleBody=std::is_same<T,float>::value;
//        y.body=int_or_float;
//    }
//    template<typename T>
//    void setWidth(T int_or_float){
//        ASSERT;
//        x.scaleBody=std::is_same<T,float>::value;
//        x.body=int_or_float;
//    }
//    float getTop(){ return y.head; }
//    float getRight(){ return x.tail; }
//    float getBottom(){ return y.tail; }
//
//    float getLeft(){ return x.head; }
//    bool isLeftScale(){ return x.scaleHead; }
//    bool isTopScale(){ return y.scaleHead; }
//    bool isBottomScale(){ return y.scaleTail; }
//    bool isRightScale(){ return x.scaleTail; }
//    bool isWidthScale(){ return x.scaleBody; }
//    bool isHeightScale(){ return y.scaleBody; }
//    void setWidthExtend(bool val){ x.extended=val; x.body=0;}
//    void setHeightExtend(bool val){ y.extended=val; y.body=0;}
//    bool getWidthExtend(){ return x.extended; }
//    bool getHeightExtend(){ return y.extended; }
//    float getMinHeight(){ return y.limit.min; }
//    float getMaxHeight(){ return y.limit.max; }
//    float getMinWidth(){ return x.limit.min; }
//    float getMaxWidth(){ return x.limit.max; }
//    void setMinHeight(float val){ y.limit.min=val; }
//    void setMaxHeight(float val){ y.limit.max=val; }
//    void setMinWidth(float val){ x.limit.min=val; }
//    void setMaxWidth(float val){ x.limit.max=val; }
//
//};
//const float Layout::empty = std::numeric_limits<float>::lowest();
//
//class Widget:public Layout {
//public:
//    HBRUSH brush;
//    Widget(){
//        brush = CreateSolidBrush(RGB(std::rand()%200,std::rand()%200,std::rand()%200));
//    }
//    virtual void render(HDC hdc) {
//        RECT r;
//        r.left=region.x;
//        r.top=region.y;
//        r.right=region.w+region.x;
//        r.bottom=region.h+region.y;
//        Rectangle(hdc,r.left,r.top,r.right,r.bottom);
//        FillRect(hdc,&r,brush);
//        //FrameRect(hdc,&r,CreateSolidBrush(RGB(0,0,0)));//RGB(std::rand()%200,std::rand()%200,std::rand()%200)));
//        if (childCount()>0) {
//            auto tmp = getfirstChild<Widget>();
//            if(tmp!=nullptr){
//                tmp->calcuRegion(this);
//                tmp->render(hdc);
//            }
//        }
//    }
//    void setChild(Widget* child){setfirstChild(child);}
//};
//
//
//
//enum class Horizontal{Left,Center,Right};
//enum class Vertical{Top,Center,Bottom};
//
//class Margin:public Widget{
//public:
//    template<typename L,typename T,typename R,typename B>
//    Margin(L left,T top,R right,B bottom){
//        setLeft(left);setTop(top);setRight(right);setBottom(bottom);
//    }
//    template<typename T>
//    Margin(T px_or_scale){
//        setLeft(px_or_scale);setTop(px_or_scale);setRight(px_or_scale);setBottom(px_or_scale);
//    }
//    template<typename T>
//    void setLeft(T int_or_float){
//        ASSERT;
//        x.head=int_or_float;
//        x.scaleHead=std::is_same<T,float>::value;
//    }
//    template<typename T>
//    void setTop(T int_or_float){
//        ASSERT;
//        y.head=int_or_float;
//        y.scaleHead=std::is_same<T,float>::value;
//    }
//    float getLeft(){ return x.head; }
//    float getTop(){ return y.head; }
//    template<typename T>
//    void setRight(T int_or_float){
//        ASSERT;
//        x.tail=int_or_float;
//        x.scaleTail=std::is_same<T,float>::value;
//    }
//    template<typename T>
//    void setBottom(T int_or_float){
//        ASSERT;
//        y.tail=int_or_float;
//        y.scaleTail=std::is_same<T,float>::value;
//    }
//    float getRight(){ return x.tail; }
//    float getBottom(){ return y.tail; }
//
//    virtual bool expandable(Axis::Enum axis)override{
//        return true;
//    }
//};
//class Extended: public Widget{
//    enum ExtendedMode{horizontal,vertical}extendedMode;
//public:
//    //TODO:把这里X Y统统去掉
//    Extended()=default;
//    template<typename X,typename T,typename B>
//    Extended(Horizontal horizontalDock, X x, T top, B bottom){
//        switch(horizontalDock){
//            case Horizontal::Left:  setLeft<X>(x);break;
//            case Horizontal::Right: setRight<X>(x);break;
//        }
//        setTop(top);
//        setBottom(bottom);
//        setWidthExtend(true);
//    }
//    template<typename X,typename H>
//    Extended(Horizontal horizontalDock, X x, H height){
//        switch(horizontalDock){
//            case Horizontal::Left:  setLeft(x);break;
//            case Horizontal::Right: setRight(x);break;
//        }
//        y.body=height;
//        y.scaleBody=std::is_same<H,float>::value;
//        setWidthExtend(true);
//    }
//
//    template<typename Y,typename L,typename R>
//    Extended(Vertical verticalDock, Y y, L left, R right){
//        switch(verticalDock){
//            case Vertical::Top:    setTop(y);break;
//            case Vertical::Bottom: setBottom(y);break;
//        }
//        setLeft(left);
//        setRight(right);
//        setHeightExtend(true);
//    }
//    template<typename Y,typename W>
//    Extended(Vertical verticalDock, Y y, W width){
//        switch(verticalDock){
//            case Vertical::Top:    setTop(y);break;
//            case Vertical::Bottom: setBottom(y);break;
//        }
//        x.body=width;
//        x.scaleBody=std::is_same<W,float>::value;
//        setHeightExtend(true);
//    }
//
//    template<typename X,typename Y>
//    Extended(Horizontal horizontalDock, Vertical verticalDock, X x, Y y){
//        switch(horizontalDock){
//            case Horizontal::Left:setLeft(x);break;
//            case Horizontal::Right:setRight(x);break;
//        }
//        switch(verticalDock){
//            case Vertical::Top:setTop(y);break;
//            case Vertical::Bottom:setBottom(y);break;
//        }
//        setHeightExtend(true);
//        setWidthExtend(true);
//    }
//    Extended(Horizontal horizontalDock, Vertical verticalDock){
//        setHeightExtend(true);
//        setWidthExtend(true);
//    }
//
//    template<typename T>
//    void setX(T int_or_float){
//        ASSERT;
//        if(x.head==Layout::empty){
//            x.tail=int_or_float;
//            x.scaleTail=std::is_same<T,float>::value;
//        }else{
//            x.head=int_or_float;
//            x.scaleHead=std::is_same<T,float>::value;
//        }
////        x.extended=false;
//    }
//    template<typename T>
//    void setY(T int_or_float){
//        ASSERT;
//        if(y.head==Layout::empty){
//            y.tail=int_or_float;
//            y.scaleTail=std::is_same<T,float>::value;
//        }else{
//            y.head=int_or_float;
//            y.scaleHead=std::is_same<T,float>::value;
//        }
////        y.extended=false;
//    }
//    //TODO:增加get/set Width Height,处理当使用Width/Height时getX getY返回不正确的结果的问题
//    float getX(){ return x.head==Layout::empty ? x.tail : x.head; }
//    float getY(){ return y.head==Layout::empty ? y.tail : y.head; }
//
//
//    virtual bool expandable(Axis::Enum axis)override{
////TODO expandable Extended
//        return true;
//    }
//
//};
//class Fixed:public Extended{
//public:
//    Fixed()=default;
//    template<typename X,typename Y,typename W,typename H>
//    Fixed(Horizontal horizontalDock,Vertical verticalDock,X x,Y y,W width,H height):
//        Extended::Extended(horizontalDock,verticalDock, x, y){
//        setWidth(width);
//        setHeight(height);
//    }
//    template<typename X,typename Y,typename W,typename H>
//    Fixed(X x,Y y,W width,H height):Extended::Extended(Horizontal::Left, Vertical::Top, x, y){
//        setWidth(width);
//        setHeight(height);
//    }
//    template<typename W,typename H>
//    Fixed(W width,H height):Extended::Extended(Horizontal::Left, Vertical::Top){
//        setWidth(width);
//        setHeight(height);
//    }
//    template<typename T>
//    void setHeight(T int_or_float){
//        ASSERT;
//        y.scaleBody=std::is_same<T,float>::value;
//        y.body=int_or_float;
//        y.extended=false;
//    }
//    template<typename T>
//    void setWidth(T int_or_float){
//        ASSERT;
//        x.scaleBody=std::is_same<T,float>::value;
//        x.body=int_or_float;
//        x.extended=false;
//    }
//    float getWidth(){ return x.body; }
//    float getHeight(){ return y.body; }
//    void setChild(Widget* widget){
//        setfirstChild(widget);
//    }
//
//    virtual bool expandable(Axis::Enum axis)override{ return false; }
//    //TODO:getChild function
//};
//
//template<typename T>
//T* limit(float maxHeight,float minHeight,float maxWidth,float minWidth,T* object){
//    object->setMinWidth(minWidth);
//    object->setMaxWidth(maxWidth);
//    object->setMinHeight(minHeight);
//    object->setMaxHeight(maxHeight);
//}
//
//
//using displayCondition = std::function<bool(Size)>;
////这个类不能为extended
//class Dynamic:public Margin{
//    bool isDefault=true;
//    struct Candidate{
//        float minW,minH;
//        Widget* widget;
//    }current;
//    std::list<Candidate> candidate;
//    float xMax=std::numeric_limits<float>::min();
//    float xMin=std::numeric_limits<float>::max();
//    float yMax=std::numeric_limits<float>::min();
//    float yMin=std::numeric_limits<float>::max();
//public:
//    Dynamic():Margin::Margin(0){}
//    void addChild(Widget* child){
//        Candidate tmp;
//        tmp.widget=child;
//        tmp.minH=child->getFillerLength(Axis::Y);
//        tmp.minW=child->getFillerLength(Axis::X);
//        xMax=std::max(xMax,tmp.minW);
//        xMin=std::min(xMin,tmp.minW);
//        x.limit.max=xMax+1;
//        x.limit.min=xMin;
//        yMax=std::max(yMax,tmp.minH);
//        yMin=std::min(yMin,tmp.minH);
//        y.limit.max=yMax+1;
//        y.limit.min=yMin;
//        candidate.push_back(tmp);
//    }
//
//    virtual bool expandable(Axis::Enum axis)override{
//        return true;
//    }
//
//    virtual void calcuRegion(Layout* container)override{
//        Layout::calcuRegion(container);
//        std::map<float,Candidate> l;
//        for(auto c:candidate){
//            l.insert(std::make_pair(region.w-c.minW+region.h-c.minH,c));
//        }
//        Widget* tmp = nullptr;
//        for(auto c:l){
//            if(c.first>0 && c.second.minW < region.w && c.second.minH < region.h){
//                tmp=c.second.widget;
//                break;
//            }
//        }
//        setfirstChild(tmp,false);
//    }
//};
//
//class Grid:public Widget{
//    std::vector<float> scaleX,scaleY;
//    std::vector<int> unitX,unitY;
//    enum adjuestMode{Scale,Unit}modeX,modeY;
//    int colm=0,rowm=0;
//    struct Container{
//        Widget* widget;
//        int col,row,span_col,span_row;
//        Container(int c,int r,int sc,int sr,Widget* w)
//            :col(c),row(r),span_col(sc),span_row(sr),widget(w){}
//    };
//    std::vector<Container> content;
//    std::vector<std::vector<int>> table;
//    int getSum(std::vector<int>&set){
//        int l=0;
//        for(int a:set)l+=a;
//        return l;
//    }
//
//    void setUnits(std::vector<int> input,adjuestMode &mode,Axis &axis,std::vector<int> &units){
//        int sum=0;
//        units.push_back(0);
//        for(int i=0;i<input.size();i++){
//            sum+=input[i];
//            units.push_back(sum);
//        }
//        axis.body=sum;
//        mode=Unit;
//    }
//    void setScales(std::vector<float> input,adjuestMode &mode,Axis &axis,std::vector<float> &scales){
//        float sum = 0;
//        scales.push_back(0);
//        for(int i=0;i<input.size();i++){
//            sum+=input[i];
//            scales.push_back(sum);
//        }
//        axis.tail=axis.head=0;
//        mode=Scale;
//    }
//    void setTable(int w,int h){
//        table.resize(h);
//        for(auto& row:table)row=std::vector<int>(w,-1);
//        colm=w;
//        rowm=h;
//    }
//public:
//
////DivideMode UnitMode ScaleMode 与 X Y排列组合
////爷吐了 Dart那种可选参数的设计简直是人类的希望
//#define CallUnitCol     setUnits(columns_length,modeX,x,unitX)
//#define CallUnitRow     setUnits(rows_length,modeY,y,unitY);
//#define CallScaleCol    setScales(columns_scales,modeX,x,scaleX)
//#define CallScaleRow    setScales(rows_scales,modeY,y,scaleY)
//#define CallDevideCol   setScales(std::vector<float>(1/columns,columns),modeX,x,scaleX)
//#define CallDevideRow   setScales(std::vector<float>(1/rows,rows),modeY,y,scaleY)
//    Grid(int rows,int columns){
//        setTable(columns,rows);
//        CallDevideRow;
//        CallDevideCol;
//    }
//    Grid(int rows,std::vector<int> columns_length){
//        setTable(columns_length.size(),rows);
//        CallDevideRow;
//        CallUnitCol;
//    }
//    Grid(int rows,std::vector<float> columns_scales){
//        setTable(columns_scales.size(),rows);
//        CallScaleCol;
//        CallDevideRow;
//    }
//
//    Grid(std::vector<int> rows_length,int columns){
//        setTable(columns,rows_length.size());
//        CallUnitRow;
//        CallDevideCol;
//    }
//    Grid(std::vector<int> rows_length,std::vector<int> columns_length){
//        for(auto ele:rows_length)std::cout<<ele<<std::endl;
//        setTable(columns_length.size(),rows_length.size());
//        CallUnitRow;
//        CallUnitCol;
//    }
//    Grid(std::vector<int> rows_length,std::vector<float> columns_scales){
//        setTable(columns_scales.size(),rows_length.size());
//        CallUnitRow;
//        CallScaleCol;
//    }
//
//    Grid(std::vector<float> rows_scales,int columns){
//        setTable(columns,rows_scales.size());
//        CallScaleRow;
//        CallDevideCol;
//    }
//    Grid(std::vector<float> rows_scales,std::vector<int> columns_length){
//        setTable(columns_length.size(),rows_scales.size());
//        CallScaleRow;
//        CallUnitCol;
//    }
//    Grid(std::vector<float> rows_scales,std::vector<float> columns_scales){
//        setTable(columns_scales.size(),rows_scales.size());
//        CallScaleRow;
//        CallScaleCol;
//    }
//
//    Grid(std::vector<int> unit_length):Grid(unit_length,unit_length){}
//    Grid(std::vector<float> unit_scale):Grid(unit_scale,unit_scale){}
//    Grid(int unit):Grid(unit,unit){}
////属实恶心
//#undef CallUnitCol
//#undef CallUnitRow
//#undef CallScaleCol
//#undef CallScaleRow
//#undef CallDevideCol
//#undef CallDevideRow
//
//    virtual void render(HDC hdc)override {
//        Widget::render(hdc);
//        for(auto c:content){
//            if(c.widget!=nullptr){
//                c.widget->calcuRegion(this);
//                c.widget->render(hdc);
//            }
//        }
//    }
//
//    void setChild(int col,int row,int spanCol,int spanRow,Widget* widget){
////        if(col>-1 && col < colm && row > -1 && row < rowm &&
////            spanCol>-1 && spanCol < colm && spanRow > -1 && spanRow < row){
//
//        Fixed* tmp=new Fixed();
//        if(modeX==Unit){
//            tmp->setLeft<int>(unitX[col]);
//            tmp->setWidth<int>(unitX[col+spanCol]-unitX[col]);
//        }
//        else {
//            tmp->setLeft<float>(scaleX[col]);
//            tmp->setWidth<float>(scaleX[col+spanCol]-scaleX[col]);
//        }
//
//        if(modeY==Unit){
//            tmp->setTop<int>(unitY[row]);
//            tmp->setHeight<int>(unitY[row+spanRow]-unitY[row]);
//        }
//        else{
//            tmp->setTop<float>(scaleY[row]);
//            tmp->setHeight<float>(scaleY[row+spanRow]-scaleY[row]);
//        }
//
//        tmp->setChild(widget);
//        content.push_back(Container(col,row,spanRow,spanCol,tmp));
//        int index=content.size()-1;
//        for(int y=row;y<row+spanRow;y++){
//            for(int x=col;x<col+spanCol;x++){
//                int &i=table[y][x];
//                if(i>-1){
//                    delete content[i].widget;
//                    content[i].widget=nullptr;
//                }
//                i=index;
//            }
//        }
//
////        }
//    }
//    void setChild(int col,int row,Widget* widget){
//        setChild(col,row,1,1,widget);
//    }
//    ~Grid(){
//        for(auto p:content)delete p.widget;
//    }
//};
////enum class AlignMode{Vertical,Horizontal,Unit};
//enum class Direction{Horizontal,Vertical};
//class Stack:public Extended{
//    //TODO:弄一个“挤出”事件
//    //AlignMode mMode;
//    Direction mFloating;
//    Horizontal hDock;
//    Vertical vDock;
//    float prv_stacking=0;
//    float max_w=0;
//    //首先此处必定只有一条轴为extended
//    virtual float getFillerLength(Axis::Enum type)override{
//        const Axis &axis = type==Axis::X ? x : y;
//        if(mFloating==Direction::Horizontal && type==Axis::Y)
//            return prv_stacking + (vDock==Vertical::Top ? y.head : y.tail);
//        else if(mFloating==Direction::Vertical && type==Axis::X)
//            return prv_stacking + (hDock==Horizontal::Left ? x.head : x.tail);
//        else
//            return limit(axis,max_w) + (axis.head==empty ? 0:axis.head) + (axis.tail==empty ? 0:axis.tail);
//    }
//public:
//    Stack(Horizontal horizontalDock,Vertical verticalDock,Direction floating)
//        :hDock(horizontalDock),vDock(verticalDock),mFloating(floating){
//        if(floating==Direction::Horizontal){
//            setLeft(0);
//            setRight(0);
//        }
//        else{
//            setTop(0);
//            setBottom(0);
//        }
//        if(horizontalDock==Horizontal::Left)x.head=0; else if(horizontalDock==Horizontal::Right) x.tail=0;
//        if(verticalDock==Vertical::Top)y.head=0; else if(verticalDock==Vertical::Bottom) y.tail=0;
//    }
//
//    Stack(Direction floating):Stack(Horizontal::Left,Vertical::Top,floating){}
//
//    virtual void calcuRegion(Layout* container)override{
//        if(childCount()==0)return;
//        bool newline=true;
//        Layout::calcuRegion(container);
//        if(mFloating==Direction::Horizontal){
//            float floating=0,stacking=0,expandCount=0,maxStacking=0;
//            std::queue<Fixed*> queue;
//            std::queue<float> floatQueue;
//            std::queue<float> stackQueue;
//            for(auto iter=begin();iter!=end()||queue.size()>0;){
//                float w,h;
//                Fixed* ptr=nullptr;
//                Widget* child=nullptr;
//                if(iter!=end()){
//                    ptr=dynamic_cast<Fixed*>(*iter);
//                    if(ptr==nullptr){iter++;continue;}
//                    child = ptr->getfirstChild<Widget>();
//                    w = child->getFillerLength(Axis::X);
//                    max_w = std::max(max_w,w);
//                    if(w==Layout::empty)w=0;
//                    h = child->getFillerLength(Axis::Y);
//                    if(h==Layout::empty) h=0;
//                    maxStacking = std::max(maxStacking,h);
//                }
//                if((floating + w > region.w || iter==end()) && newline==false){
//                    float expandPx = (region.w-(int)floating)/expandCount;
//                    expandCount=0;
//                    floating=0;
//                    while(queue.size()>0){
//                        Fixed* front = queue.front();
//                        queue.pop();
//                        front->setLeft<int>(floating);
//                        front->setTop<int>(stacking);
//                        w = floatQueue.front();
//                        h = stackQueue.front();
//                        stackQueue.pop();
//                        floatQueue.pop();
//                        child = front->getfirstChild<Widget>();
//                        if(child->expandable(Axis::X)) w+=expandPx;
//                        if(child->getMinWidth()!=Layout::empty)
//                            w = std::min(child->getMaxWidth(),w);
//                        front->setWidth<int>(w);
//                        front->setHeight<int>(maxStacking);
//                        front->calcuRegion(this);
//                        floating+=w;
//                    }
//                    floating=0;
//                    stacking+=maxStacking;
//                    maxStacking=0;
//                    newline=true;
//                }
//                else{
//                    if(child->expandable(Axis::X))expandCount++;
//                    floatQueue.push(w);
//                    stackQueue.push(h);
//                    queue.push(ptr);
//                    floating+=w;
//                    iter++;
//                    newline=false;
//                }
//            }
//            region.h=stacking;
//            prv_stacking=stacking;
//        }
//        else{
//
//        }
//    }
//
//    void Add(Widget* child){
//        auto ptr = new Fixed(0,0);
//        ptr->setChild(child);
//        add(ptr);
//    }
//    virtual void render(HDC hdc)override {
//        Widget::render(hdc);
//        foreach<Widget>([&](Widget& child){
//            child.calcuRegion(this);
//            child.render(hdc);
//        });
//    }
//    virtual bool expandable(Axis::Enum type)override{
//        if(mFloating==Direction::Horizontal && type==Axis::X)return true;
//        else if(mFloating==Direction::Vertical && type==Axis::Y)return true;
//        else return false;
//    }
//};
//
//class Flow:public Widget{
//
//};
//
//class Panel:public Widget{
//public:
//    std::list<Widget*> childs;
//    virtual void render(HDC hdc)override {
//        Widget::render(hdc);
//        for(auto c:childs){
//            if(c==nullptr)continue;
//            c->calcuRegion(this);
//            c->render(hdc);
//        }
//    }
//};
//
////TODO:将上面的类改成Grid<Fixed>形式
////TODO:添加Grid<Extended>,该类setChild只接受Fixed和Extend类型
////TODO:添加Grid<Margin>
////TODO:Stack:Extend布局类 Stack<Fixed> Stack<Margin>
//
////TODO:考虑用Fixed Extended Margin 套 Grid Panel Stack的方式而非模板特化！



#undef ASSERT
#endif
