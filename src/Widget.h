#ifndef KAT_LAYOUT
#define KAT_LAYOUT
#include <functional>
#include <list>
#include <typeindex>
#include <memory>
#include <string>
#include<limits>
#include"../cpp_DOM/src/DOM.h"

#include<windows.h>

struct absRegion
{
	float x = 0, y = 0, w = 0, h = 0;
};

struct Size
{
	int height, width;
	float scale_height, scale_width;
};

class Widget;
class Layout
{
public:
    std::string id;
    Layout(){
        x.type=Axis::X;
        y.type=Axis::Y;
    }
	Layout* child=nullptr;
	struct Region{
	    const float obsolete=std::numeric_limits<int>::lowest();
	    float x,y,w,h,r,b;
        void setObsolete(){
//            x=y=w=h=r=b=obsolete;
        }
	}region;
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
public:
    virtual float getExetndedParentFiller(Axis::Enum type){
        float ans = 0;
        const Axis &axis = type==Axis::X ? x : y;
        //蛤？父类长宽由我定？
        if((axis.extended||axis.scaleBody||axis.scaleHead||axis.scaleTail)){
            //我也不知道啊
            if(child!=nullptr)  /*去我子类那问吧*/
                ans = child->getExetndedParentFiller(type);
            else                /*草（日语） 我没子类了*/
                throw "无法计算坐标";
        }
        else{
            //我知道
            ans = limit(axis,axis.body);
        }
        return ans + (axis.head==empty ? 0:axis.head) + (axis.tail==empty ? 0:axis.tail);//加上缝隙
    }
private:
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
    }
public:
    void calcuRegion(Layout* container){
        child->region.setObsolete();
        calcuAxis(x,region.x,region.w,region.r,container->region.x,container->region.w);
        calcuAxis(y,region.y,region.h,region.b,container->region.y,container->region.h);
	}
};
const float Layout::empty = std::numeric_limits<float>::lowest();

class Widget:public Layout {
public:
    virtual void render(HDC hdc) {
        Rectangle(hdc, region.x, region.y, region.x + region.w, region.y + region.h);
        if (child != nullptr) {
            child->calcuRegion(this);
            ((Widget*)child)->render(hdc);
        }
    }
};
//
//Widget *Widget::Zero = new Widget();
//
//using displayCondition = std::function<bool(Size)>;
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

#endif
