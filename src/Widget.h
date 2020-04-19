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
    virtual void render(HDC hdc) {
        RECT r;
        r.left=region.x;
        r.top=region.y;
        r.right=region.w+region.x;
        r.bottom=region.h+region.y;

        FrameRect(hdc,&r,CreateSolidBrush(RGB(std::rand()%200,std::rand()%200,std::rand()%200)));
        if (child != nullptr) {
            child->calcuRegion(this);
            ((Widget*)child)->render(hdc);
        }
    }
};

enum Dock{Left=1,Top=1<<1,Right=1<<2,Bottom=1<<3};

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

class Extend:public Widget{
public:
    Extend()=default;
    template<typename X,typename Y>
    Extend(int dockEdge,X x,Y y){
        if((dockEdge&Dock::Left)==Dock::Left)setLeft(x);
        if((dockEdge&Dock::Top)==Dock::Top)setTop(y);
        if((dockEdge&Dock::Right)==Dock::Right)setRight(x);
        if((dockEdge&Dock::Bottom)==Dock::Bottom)setBottom(y);
        setHeightExtend(true);
        setWidthExtend(true);
    }
    Extend(int dockEdge){
        Extend(dockEdge,0,0);
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
        x.extended=false;
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
        y.extended=false;
    }
    float getX(){ return x.head==Layout::empty ? x.tail : x.head; }
    float getY(){ return y.head==Layout::empty ? y.tail : y.head; }
};

class Fixed:public Extend{
public:
    Fixed()=default;
    template<typename X,typename Y,typename W,typename H>
    Fixed(int dockEdge,X x,Y y,W width,H height):Extend::Extend(dockEdge,x,y){
        setWidth(width);
        setHeight(height);
    }
    template<typename X,typename Y,typename W,typename H>
    Fixed(X x,Y y,W width,H height):Extend::Extend(Dock::Left+Dock::Top,x,y){
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
//TODO:将上面的类改成Grid<Fixed>形式
//TODO:添加Grid<Extend>,该类setChild只接受Fixed和Extend类型
//TODO:添加Grid<Margin>
//TODO:Stack:Extend布局类 Stack<Fixed> Stack<Margin>

//TODO:考虑用Fixed Extend Margin 套 Grid Panel Stack的方式而非模板特化！



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



//
//Widget *Widget::Zero = new Widget();
//

//
//class MutiWidget : public Widget
//{
//	displayCondition smaller_condition, larger_condition;
//	Widget *smaller,*larger,*normal;
//	virtual void resetRegion(Widget *parent) override
//	{
//        //选择合适的layout
//        Size size;
//        bool flag = false;
//        size.height = region.h;
//        size.width = region.w;
//        size.scale_height = region.h / parent->region.h;
//        size.scale_width = region.w / parent->region.w;
//        if (larger_condition)
//        {
//            if (larger_condition(size))
//            {
//                flag = true;
//                larger->resetRegion(parent);
//            }
//        }
//        if (smaller_condition)
//        {
//            if (smaller_condition(size))
//            {
//                flag = true;
//                smaller->resetRegion(parent);
//            }
//        }
//        if (!flag)normal->resetRegion(parent);;
//	}
//
//public:
//	MutiWidget(Widget* smaller, displayCondition smaller_condition, Widget* normal,
//			   Widget* larger, displayCondition larger_condition)
//	{
//
//		this->smaller = smaller;
//		this->larger = larger;
//		this->larger_condition = larger_condition;
//		this->smaller_condition = smaller_condition;
//	}
//	MutiWidget(Widget* smaller, displayCondition smaller_condition, Widget* normal)
//	{
//		this->normal = normal;
//		this->smaller = smaller;
//		this->smaller_condition = smaller_condition;
//	}
//	MutiWidget(Widget* normal, Widget* larger, displayCondition larger_condition)
//	{
//		this->normal = normal;
//		this->larger = larger;
//		this->larger_condition = larger_condition;
//	}
//
//	//void setSmallerChild(Layout* layout, displayCondition condition) {
//	//	if (smaller_layout != nullptr)delete smaller_layout;
//	//	this->smaller_layout = layout;
//	//	//if(layout!=nullptr)layout->parent=this;
//	//	this->smaller_condition = condition;
//	//	//添加region到脏矩形
//	//	//resetRegion
//	//	//添加region到脏矩形
//	//	//....
//	//	//通知刷新
//	//}
//
//	//void setLargerChild(Layout* layout, displayCondition condition) {
//	//	if (larger_layout != nullptr)delete larger_layout;
//	//	this->larger_layout = layout;
//	//	//if (layout != nullptr)layout->parent=this;
//	//	this->larger_condition = condition;
//	//	//添加region到脏矩形
//	//	//resetRegion
//	//	//添加region到脏矩形
//	//	//....
//	//	//通知刷新
//	//}
//};
//
//class ScrollField:public Widget
//{
//public:
//    ScrollField(){}
//
//};
//
//class Canvas : public Widget
//{
//
//};
//
//class Stack : public Widget
//{
//
//};
//
//class Grid:public Widget
//{
//    int colm,rowm;
//    struct Container
//    {
//        Widget* widget;
//        Container(int row,int col,int spanRow,int spanCol,Widget* widget):
//        row(row),col(col),spanRow(spanRow),spanCol(spanCol){
//            this->widget->child=widget;
//            widget->left=new int(0);
//            widget->top=new int(0);
//            widget->width=new int(0);
//            widget->height=new int(0);
//        }
//        int row,col,spanRow,spanCol;
//    };
//    std::vector<Container*> content;
//    std::vector<std::vector<int>> table;
//    Container* get(int col,int row) {
//        return content[table[col][row]];
//    }
//public:
//    virtual void render(HDC hdc)override
//    {
//        Rectangle(hdc,region.x,region.y,region.x+region.w,region.y+region.h);
//        for(auto c:content)
//        {
//            if(c!=nullptr)
//            {
//                c->widget->resetRegion(this);
//                c->widget->render(hdc);
//            }
//        }
//    }
//    void resetRegion(Widget* parent)override
//    {
//        Widget::resetRegion(parent);
//        int unitW= region.w / colm;
//        int unitH= region.h / rowm;
//        for(auto c:content)
//        {
//            Layout* d=c->widget;
//            *d->left=c->col*unitW;
//            *d->top=c->row*unitH;
//            *d->width=c->spanCol*unitW;
//            *d->height=c->spanRow*unitH;
//        }
//    }
//
//    Grid(int columns,int rows,DOM::initializer property)
//    {
//        colm=columns;
//        rowm=rows;
//        DOM::moveProperty(property,this);
//
//        table.resize(rows);
//        for (int i = 0; i < rows; i++)
//            table[i].resize(columns);
//    }
//
//    void setChild(int col,int row,int spanCol,int spanRow,Widget* widget)
//    {
//        if(col>-1 && col < colm && row > -1 && row < rowm &&
//            spanCol>-1 && spanCol < colm && spanRow > -1 && spanRow < row)
//        {
//            content.push_back(new Container(col,row,spanRow,spanCol,widget));
//            int index=content.size();
//            for(int y=col;y<col+spanCol;y++)
//                for(int x=row;x<row+spanRow;x++)
//                {
//                    delete get(x,y)->widget;
//                    delete get(x,y);
//                    content[table[x][y]]=nullptr;
//                    table[y][x]=index;
//                }
//
//        }
//    }
//
//    void setChild(int col,int row,Widget* widget)
//    {
//        setChild(col,row,1,1,widget);
//    }
//
//};

//namespace DOM
//{
//	auto child(Widget *layout) { return item(&Layout::child, layout); }
//	auto extendedWidth(){return item(&Layout::pending_width,true);}
//	auto extendedHeight(){return item(&Layout::pending_height,true);}
//
//	auto left(int px) { return item(&Layout::left, new int(px)); }
//	auto top(int px) { return item(&Layout::top, new int(px)); }
//	auto right(int px) { return item(&Layout::right, new int(px)); }
//	auto bottom(int px) { return item(&Layout::bottom, new int(px)); }
//	auto width(int px) { return item(&Layout::width, new int(px)); }
//	auto height(int px) { return item(&Layout::height, new int(px)); }
//	auto maxWidth(int px){ return item(&Layout::max_width,new int(px));}
//	auto minWidth(int px){ return item(&Layout::min_width,new int(px));}
//	auto maxHeight(int px){ return item(&Layout::max_height,new int(px));}
//	auto minHeight(int px){ return item(&Layout::min_height,new int(px));}
//
//	auto margin(int left,int top,int right,int bottom){
//	    return new ComplexItem<Layout>([=](Layout* it){
//	        it->left=new int(left);
//	        it->top=new int(top);
//	        it->right=new int(right);
//	        it->bottom=new int(bottom);
//	    });
//	}
//	auto margin(int px){
//	    return margin(px,px,px,px);
//	}
//
//	auto left(float scale) { return item(&Layout::scale_left, new float(scale)); }
//	auto top(float scale) { return item(&Layout::scale_top, new float(scale)); }
//	auto right(float scale) { return item(&Layout::scale_right, new float(scale)); }
//	auto bottom(float scale) { return item(&Layout::scale_bottom, new float(scale)); }
//	auto width(float scale) { return item(&Layout::scale_width, new float(scale)); }
//	auto height(float scale) { return item(&Layout::scale_height, new float(scale)); }
//	auto maxWidth(float scale){ return item(&Layout::scale_max_width,new float(scale));}
//	auto minWidth(float scale){ return item(&Layout::scale_min_width,new float(scale));}
//	auto maxHeight(float scale){ return item(&Layout::scale_max_height,new float(scale));}
//	auto minHeight(float scale){ return item(&Layout::scale_min_height,new float(scale));}
//
//	auto child(int col,int row,int spanCol,int spanRow,Widget* widget)
//    {
//	    return new ComplexItem<Grid>([=](Grid* it){
//	       it->setChild(col,row,spanCol,spanRow,widget);
//	    });
//    }
//
//    auto child(int col,int row,Widget* widget)
//    {
//        return new ComplexItem<Grid>([=](Grid* it){
//            it->setChild(col,row,widget);
//        });
//    }
//
//}
#undef ASSERT
#endif
