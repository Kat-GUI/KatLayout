#ifndef KAT_LAYOUT
#define KAT_LAYOUT
#include <functional>
#include <list>
#include <typeindex>
#include <memory>
#include <string>
#include<limits>
#include<vector>
#include<math.h>
#include<iostream>
//#include"../cpp_DOM/src/DOM.h"

#include<windows.h>
struct Size
{
	int height, width;
	float scale_height, scale_width;
};

class Widget;
class Layout
{
public:
    Layout(){
        x.type=Axis::X;
        y.type=Axis::Y;
    }
    std::string id;
    Layout* child=nullptr;
    struct Region{
        const float obsolete=std::numeric_limits<int>::lowest();
        float x,y,w,h,r,b;
        void setObsolete(){
//            x=y=w=h=r=b=obsolete;
        }
    }region;
protected:

    static const float empty;
	struct Axis{
	    enum Enum{X,Y}type;
	    float head=empty;
	    float body=empty;
	    float tail=empty;
	    bool scaleHead=false,scaleBody=false,scaleTail=false;
	    bool extended=false;//为真时，body也要设为非empty
	    struct Limit{
	        float max=empty;
	        float min=empty;
	    }limit;
	}x,y;

private:
    float limit(const Axis &axis,const float& distance){
        float ans=distance;
        if(axis.limit.max!=empty && distance > axis.limit.max)ans=axis.limit.max;
        if(axis.limit.min!=empty && distance < axis.limit.min)ans=axis.limit.min;
        return ans;
    }
protected:
    virtual float getExetndedParentFiller(Axis::Enum type){
        float ans = 0;
        const Axis &axis = type==Axis::X ? x : y;
        //蛤？父类长宽由我定？
        if((axis.extended||axis.scaleBody||axis.scaleHead||axis.scaleTail)){
            //我也不知道啊
            if(child!=nullptr)  //去我子类那问吧
                ans = child->getExetndedParentFiller(type);
            else                //草（日语） 我没子类了
                throw "无法计算坐标";
        }
        else{
            //我知道
            ans = limit(axis,axis.body);
        }
        return ans + (axis.head==empty ? 0:axis.head) + (axis.tail==empty ? 0:axis.tail);//加上缝隙
    }
protected:
    void calcuAxis(Axis &axis,float &begin,float &distance,float &end,
            const float& parent_begin,const float& parent_distance){
        float tmp;
        //直接穷举 通俗易懂
        if(axis.head==empty && axis.body!=empty && axis.tail==empty){
            //[   ],body,[   ]
            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
            distance = limit(axis,axis.extended ? child->getExetndedParentFiller(axis.type): tmp);
            begin = (parent_distance - distance)/2;
            end = begin;
        }
        else if(axis.head==empty && axis.body!=empty && axis.tail!=empty){
            //[   ],body,tail
            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
            distance = limit(axis,axis.extended ? child->getExetndedParentFiller(axis.type): tmp);
            end = axis.scaleTail ? axis.tail*parent_distance : axis.tail;
            begin = parent_distance - distance - end;
        }
        else if(axis.head != empty && axis.body!=empty && axis.tail==empty){
            //head,body,[   ]
            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
            distance = limit(axis,axis.extended ? child->getExetndedParentFiller(axis.type): tmp);
            begin = axis.scaleHead ? axis.head*parent_distance : axis.head;
            end = parent_distance - distance - begin;
        }
        else if(axis.head!=empty && axis.body==empty && axis.tail!=empty){
            //head,[   ],tail
            begin = axis.scaleHead ? axis.head*parent_distance : axis.head;
            end = axis.scaleTail ? axis.tail*parent_distance : axis.tail;
            tmp = parent_distance - begin - end;
            distance = limit(axis,axis.extended ? child->getExetndedParentFiller(axis.type): tmp);
            if(tmp!=distance) {
                begin += (tmp - distance)/2;
                end += (tmp - distance)/2;
            }
        }else{
            //避免非parent::extended时执行此代码，否则会有意外的结果
            begin=axis.head;
            end = axis.tail;
            distance=limit(axis,axis.extended ? child->getExetndedParentFiller(axis.type): axis.body);
        }
        begin+=parent_begin;
        end+=parent_begin;
        //TODO:安排整体缩放
    }
public:
    virtual void calcuRegion(Layout* container){
        child->region.setObsolete();
        calcuAxis(x,region.x,region.w,region.r,container->region.x,container->region.w);
        calcuAxis(y,region.y,region.h,region.b,container->region.y,container->region.h);
	}

	//TODO: 把这里set get都删掉
	//下面到undef之前都是烦人的口水代码
#define ASSERT static_assert(std::is_same<T,int>::value||std::is_same<T,float>::value||std::is_same<T,double>::value,"The template parameter must be int or float.")
	template<typename T>
	void setLeft(T int_or_float){
        ASSERT;
        x.scaleHead=std::is_same<T,float>::value;
        x.head=int_or_float;
    }
    template<typename T>
    void setTop(T int_or_float){
        ASSERT;
        y.scaleHead=std::is_same<T,float>::value;
        y.head=int_or_float;
    }
    template<typename T>
    void setRight(T int_or_float){
        ASSERT;
        x.scaleTail=std::is_same<T,float>::value;
        x.tail=int_or_float;
    }
    template<typename T>
    void setBottom(T int_or_float){
        ASSERT;
        y.scaleTail=std::is_same<T,float>::value;
        y.tail=int_or_float;
    }
    template<typename T>

    float getTop(){ return y.head; }
    float getRight(){ return x.tail; }
    float getBottom(){ return y.tail; }

    float getLeft(){ return x.head; }
    bool isLeftScale(){ return x.scaleHead; }
    bool isTopScale(){ return y.scaleHead; }
    bool isBottomScale(){ return y.scaleTail; }
    bool isRightScale(){ return x.scaleTail; }
    bool isWidthScale(){ return x.scaleBody; }
    bool isHeightScale(){ return y.scaleBody; }
    void setWidthExtend(bool val){ x.extended=val; x.body=0;}
    void setHeightExtend(bool val){ y.extended=val; y.body=0;}
    bool getWidthExtend(){ return x.extended; }
    bool getHeightExtend(){ return y.extended; }

};
const float Layout::empty = std::numeric_limits<float>::lowest();

class Widget:public Layout {
public:
    HBRUSH brush;
    Widget(){
        brush = CreateSolidBrush(RGB(std::rand()%200,std::rand()%200,std::rand()%200));
    }
    virtual void render(HDC hdc) {
        RECT r;
        r.left=region.x;
        r.top=region.y;
        r.right=region.w+region.x;
        r.bottom=region.h+region.y;
        Rectangle(hdc,r.left,r.top,r.right,r.bottom);
        FillRect(hdc,&r,brush);
        //FrameRect(hdc,&r,CreateSolidBrush(RGB(0,0,0)));//RGB(std::rand()%200,std::rand()%200,std::rand()%200)));
        if (child != nullptr) {
            child->calcuRegion(this);
            ((Widget*)child)->render(hdc);
        }
    }
};

enum class Horizontal{Left,Center,Right};
enum class Vertical{Top,Center,Bottom};

class Margin:public Widget{
public:
    template<typename L,typename T,typename R,typename B>
    Margin(L left,T top,R right,B bottom){
        setLeft(left);setTop(top);setRight(right);setBottom(bottom);
    }
    template<typename T>
    Margin(T px_or_scale){
        setLeft(px_or_scale);setTop(px_or_scale);setRight(px_or_scale);setBottom(px_or_scale);
    }
    template<typename T>
    void setLeft(T int_or_float){
        ASSERT;
        x.head=int_or_float;
        x.scaleHead=std::is_same<T,float>::value;
    }
    template<typename T>
    void setTop(T int_or_float){
        ASSERT;
        y.head=int_or_float;
        y.scaleHead=std::is_same<T,float>::value;
    }
    float getLeft(){ return x.head; }
    float getTop(){ return y.head; }
    template<typename T>
    void setRight(T int_or_float){
        ASSERT;
        x.tail=int_or_float;
        x.scaleTail=std::is_same<T,float>::value;
    }
    template<typename T>
    void setBottom(T int_or_float){
        ASSERT;
        y.tail=int_or_float;
        y.scaleTail=std::is_same<T,float>::value;
    }
    float getRight(){ return x.tail; }
    float getBottom(){ return y.tail; }
};
class Extended: public Widget{
public:
    Extended()=default;
    template<typename X,typename T,typename B>
    Extended(Horizontal horizontalDock, X x, T top, B bottom){
        switch(horizontalDock){
            case Horizontal::Left:  setLeft(x);break;
            case Horizontal::Right: setRight(x);break;
        }
        setTop(top);
        setBottom(bottom);
        setWidthExtend(true);
    }
    template<typename X,typename H>
    Extended(Horizontal horizontalDock, X x, H height){
        switch(horizontalDock){
            case Horizontal::Left:  setLeft(x);break;
            case Horizontal::Right: setRight(x);break;
        }
        y.body=height;
        y.scaleBody=std::is_same<H,float>::value;
        setWidthExtend(true);
    }

    template<typename Y,typename L,typename R>
    Extended(Vertical verticalDock, Y y, L left, R right){
        switch(verticalDock){
            case Vertical::Top:    setTop(x);break;
            case Vertical::Bottom: setBottom(x);break;
        }
        setLeft(left);
        setRight(right);
        setHeightExtend(true);
    }
    template<typename Y,typename W>
    Extended(Vertical verticalDock, Y y, W width){
        switch(verticalDock){
            case Vertical::Top:    setTop(y);break;
            case Vertical::Bottom: setBottom(y);break;
        }
        x.body=width;
        x.scaleBody=std::is_same<W,float>::value;
        setHeightExtend(true);
    }

    template<typename X,typename Y>
    Extended(Horizontal horizontalDock, Vertical verticalDock, X x, Y y){
        switch(horizontalDock){
            case Horizontal::Left:setLeft(x);break;
            case Horizontal::Right:setRight(x);break;
        }
        switch(verticalDock){
            case Vertical::Top:setTop(y);break;
            case Vertical::Bottom:setBottom(y);break;
        }
        setHeightExtend(true);
        setWidthExtend(true);
    }
    Extended(Horizontal horizontalDock, Vertical verticalDock){
        setHeightExtend(true);
        setWidthExtend(true);
    }

    template<typename T>
    void setX(T int_or_float){
        ASSERT;
        if(x.head==Layout::empty){
            x.tail=int_or_float;
            x.scaleTail=std::is_same<T,float>::value;
        }else{
            x.head=int_or_float;
            x.scaleHead=std::is_same<T,float>::value;
        }
//        x.extended=false;
    }
    template<typename T>
    void setY(T int_or_float){
        ASSERT;
        if(y.head==Layout::empty){
            y.tail=int_or_float;
            y.scaleTail=std::is_same<T,float>::value;
        }else{
            y.head=int_or_float;
            y.scaleHead=std::is_same<T,float>::value;
        }
//        y.extended=false;
    }
    //TODO:增加get/set Width Height,处理当使用Width/Height时getX getY返回不正确的结果的问题
    float getX(){ return x.head==Layout::empty ? x.tail : x.head; }
    float getY(){ return y.head==Layout::empty ? y.tail : y.head; }
};
class Fixed:public Extended{
public:
    Fixed()=default;
    template<typename X,typename Y,typename W,typename H>
    Fixed(int dockEdge,X x,Y y,W width,H height):Extended::Extended(dockEdge, x, y){
        setWidth(width);
        setHeight(height);
    }
    template<typename X,typename Y,typename W,typename H>
    Fixed(X x,Y y,W width,H height):Extended::Extended(Horizontal::Left, Vertical::Top, x, y){
        setWidth(width);
        setHeight(height);
    }
    template<typename W,typename H>
    Fixed(W width,H height):Extended::Extended(Horizontal::Left, Vertical::Top){
        setWidth(width);
        setHeight(height);
    }
    template<typename T>
    void setHeight(T int_or_float){
        ASSERT;
        y.scaleBody=std::is_same<T,float>::value;
        y.body=int_or_float;
        y.extended=false;
    }
    template<typename T>
    void setWidth(T int_or_float){
        ASSERT;
        x.scaleBody=std::is_same<T,float>::value;
        x.body=int_or_float;
        x.extended=false;
    }
    float getWidth(){ return x.body; }
    float getHeight(){ return y.body; }
};

using displayCondition = std::function<bool(Size)>;
//这个类不能为extended
class Dynamic:public Fixed{
    bool isDefault=true;
public:
    std::list<std::pair<Widget*,displayCondition>> candidate;
    const displayCondition empty;
    const displayCondition caseElse= [&](Size){ isDefault=true;return true;};
    virtual void calcuRegion(Layout* container)override{
        Layout::calcuRegion(container);
        Size size;
        size.width=region.w;
        size.height=region.h;
        size.scale_height=region.h/container->region.h;
        size.scale_width=region.w/container->region.w;
        isDefault=false;
        bool flag=true;
        Widget* default_widget=nullptr;
        for(auto p:candidate){
            if(p.second(size)){
                if(isDefault){
                    default_widget=p.first;
                    continue;
                }
                child=p.first;
                flag=false;
            }
        }
        if(flag)child=default_widget;
    }
};

class Grid:public Widget{
    std::vector<float> scaleX,scaleY;
    std::vector<int> unitX,unitY;
    enum adjuestMode{Scale,Unit}modeX,modeY;
    int colm=0,rowm=0;
    struct Container{
        Widget* widget;
        int col,row,span_col,span_row;
        Container(int c,int r,int sc,int sr,Widget* w)
            :col(c),row(r),span_col(sc),span_row(sr),widget(w){}
    };
    std::vector<Container> content;
    std::vector<std::vector<int>> table;
    int getSum(std::vector<int>&set){
        int l=0;
        for(int a:set)l+=a;
        return l;
    }

    void setUnits(std::vector<int> input,adjuestMode &mode,Axis &axis,std::vector<int> &units){
        int sum=0;
        units.push_back(0);
        for(int i=0;i<input.size();i++){
            sum+=input[i];
            units.push_back(sum);
        }
        axis.body=sum;
        mode=Unit;
    }
    void setScales(std::vector<float> input,adjuestMode &mode,Axis &axis,std::vector<float> &scales){
        float sum = 0;
        scales.push_back(0);
        for(int i=0;i<input.size();i++){
            sum+=input[i];
            scales.push_back(sum);
        }
        axis.tail=axis.head=0;
        mode=Scale;
    }
    void setTable(int w,int h){
        table.resize(h);
        for(auto& row:table)row=std::vector<int>(w,-1);
        colm=w;
        rowm=h;
    }
public:

//DivideMode UnitMode ScaleMode 与 X Y排列组合
//爷吐了 Dart那种可选参数的设计简直是人类的希望
#define CallUnitCol     setUnits(columns_length,modeX,x,unitX)
#define CallUnitRow     setUnits(rows_length,modeY,y,unitY);
#define CallScaleCol    setScales(columns_scales,modeX,x,scaleX)
#define CallScaleRow    setScales(rows_scales,modeY,y,scaleY)
#define CallDevideCol   setScales(std::vector<float>(1/columns,columns),modeX,x,scaleX)
#define CallDevideRow   setScales(std::vector<float>(1/rows,rows),modeY,y,scaleY)
    Grid(int rows,int columns){
        setTable(columns,rows);
        CallDevideRow;
        CallDevideCol;
    }
    Grid(int rows,std::vector<int> columns_length){
        setTable(columns_length.size(),rows);
        CallDevideRow;
        CallUnitCol;
    }
    Grid(int rows,std::vector<float> columns_scales){
        setTable(columns_scales.size(),rows);
        CallScaleCol;
        CallDevideRow;

    }

    Grid(std::vector<int> rows_length,int columns){
        setTable(columns,rows_length.size());
        CallUnitRow;
        CallDevideCol;
    }
    Grid(std::vector<int> rows_length,std::vector<int> columns_length){
        for(auto ele:rows_length)std::cout<<ele<<std::endl;
        setTable(columns_length.size(),rows_length.size());
        CallUnitRow;
        CallUnitCol;
    }
    Grid(std::vector<int> rows_length,std::vector<float> columns_scales){
        setTable(columns_scales.size(),rows_length.size());
        CallUnitRow;
        CallScaleCol;
    }

    Grid(std::vector<float> rows_scales,int columns){
        setTable(columns,rows_scales.size());
        CallScaleRow;
        CallDevideCol;
    }
    Grid(std::vector<float> rows_scales,std::vector<int> columns_length){
        setTable(columns_length.size(),rows_scales.size());
        CallScaleRow;
        CallUnitCol;
    }
    Grid(std::vector<float> rows_scales,std::vector<float> columns_scales){
        setTable(columns_scales.size(),rows_scales.size());
        CallScaleRow;
        CallScaleCol;
    }

    Grid(std::vector<int> unit_length):Grid(unit_length,unit_length){}
    Grid(std::vector<float> unit_scale):Grid(unit_scale,unit_scale){}
    Grid(int unit):Grid(unit,unit){}
//属实恶心
#undef CallUnitCol
#undef CallUnitRow
#undef CallScaleCol
#undef CallScaleRow
#undef CallDevideCol
#undef CallDevideRow

    virtual void render(HDC hdc)override {
        Widget::render(hdc);
        for(auto c:content){
            if(c.widget!=nullptr){
                c.widget->calcuRegion(this);
                c.widget->render(hdc);
            }
        }
    }

    void setChild(int col,int row,int spanCol,int spanRow,Widget* widget){
//        if(col>-1 && col < colm && row > -1 && row < rowm &&
//            spanCol>-1 && spanCol < colm && spanRow > -1 && spanRow < row){

        Fixed* tmp=new Fixed();
        if(modeX==Unit){
            tmp->setLeft<int>(unitX[col]);
            tmp->setWidth<int>(unitX[col+spanCol]-unitX[col]);
        }
        else {
            tmp->setLeft<float>(scaleX[col]);
            tmp->setWidth<float>(scaleX[col+spanCol]-scaleX[col]);
        }

        if(modeY==Unit){
            tmp->setTop<int>(unitY[row]);
            tmp->setHeight<int>(unitY[row+spanRow]-unitY[row]);
        }
        else{
            tmp->setTop<float>(scaleY[row]);
            tmp->setHeight<float>(scaleY[row+spanRow]-scaleY[row]);
        }

        tmp->child=widget;
        content.push_back(Container(col,row,spanRow,spanCol,tmp));
        int index=content.size()-1;
        for(int y=row;y<row+spanRow;y++){
            for(int x=col;x<col+spanCol;x++){
                int &i=table[y][x];
                if(i>-1){
                    delete content[i].widget;
                    content[i].widget=nullptr;
                }
                i=index;
            }
        }

//        }
    }
    void setChild(int col,int row,Widget* widget){
        setChild(col,row,1,1,widget);
    }
    ~Grid(){
        for(auto p:content)delete p.widget;
    }
};
//enum class AlignMode{Vertical,Horizontal,Unit};
enum class Direction{Horizontal,Vertical};
class Stack:public Margin{
    std::list<Extended*> childs;
    //AlignMode mMode;
    Direction mFloating;
    Horizontal hDock;
    Vertical vDock;
    void AddChild(Widget* c){
        auto tmp = new Extended(hDock,vDock,0,0);
        tmp->child=c;
        childs.push_back(tmp);
    }
public:
    Stack(Horizontal horizontalDock,Vertical verticalDock,Direction floating)
        :hDock(horizontalDock),vDock(verticalDock),mFloating(floating),Margin::Margin(0){}

    Stack(Direction floating):Stack(Horizontal::Left,Vertical::Top,floating){}

    virtual void calcuRegion(Layout* container)override{
        Layout::calcuRegion(container);
        int floating=0,stacking=0,maxStacking=0;
        if(mFloating==Direction::Horizontal){
            for(std::list<Extended*>::iterator iter=childs.begin();iter!=childs.end();){
                (*iter)->setX<int>(floating+region.x);
                (*iter)->setY<int>(stacking+region.y);
                (*iter)->calcuRegion(this);
                if(floating != 0 && floating + (*iter)->region.w > region.w){
                    stacking+=maxStacking;
                    floating=maxStacking=0;
                }
                else{
                    maxStacking=std::max(maxStacking,(int)(*iter)->region.h);
                    floating+=(*iter)->region.w;
                    iter++;
                }
            }
            region.h=stacking;
        }
        else{
            for(std::list<Extended*>::iterator iter=childs.begin();iter!=childs.end();){
                (*iter)->setY<int>(floating+region.y);
                (*iter)->setX<int>(stacking+region.x);
                (*iter)->calcuRegion(this);
                if(floating != 0 && floating + (*iter)->region.h > region.h){
                    stacking+=maxStacking;
                    floating=maxStacking=0;
                }
                else{
                    maxStacking=std::max(maxStacking,(int)(*iter)->region.w);
                    floating+=(*iter)->region.h;
                    iter++;
                }
            }
            region.h=stacking;
        }

    }
    //拒绝Margin类型
    void Add(Extended* child){ AddChild(child); }
    void Add(Fixed* child){ AddChild(child); }

    virtual void render(HDC hdc)override {
        Widget::render(hdc);
        for(auto c:childs){
            c->render(hdc);
        }
    }
};

class Panel:public Widget{
public:
    std::list<Widget*> childs;
    virtual void render(HDC hdc)override {
        Widget::render(hdc);
        for(auto c:childs){
            if(c==nullptr)continue;
            c->calcuRegion(this);
            c->render(hdc);
        }
    }
};

//TODO:将上面的类改成Grid<Fixed>形式
//TODO:添加Grid<Extended>,该类setChild只接受Fixed和Extend类型
//TODO:添加Grid<Margin>
//TODO:Stack:Extend布局类 Stack<Fixed> Stack<Margin>

//TODO:考虑用Fixed Extended Margin 套 Grid Panel Stack的方式而非模板特化！



#undef ASSERT
#endif
