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
    Layout(){
        //region.setObsolete();
    }
	Layout* child;
	struct Region{
	    const float obsolete=std::numeric_limits<int>::lowest();
	    float x,y,w,h,r,b;
        void setObsolete(){
            x=y=w=h=r=b=obsolete;
        }
	}region;
    static const float empty;
	struct Axis{
	    float head=empty;
	    float body=empty;
	    float tail=empty;
	    bool scaleHead,scaleBody,scaleTail;
	    bool extended=false;//为真时，body也要设为非empty
	    struct Limit{
	        float max=empty;
	        float min=empty;
	    }limit;
	}x,y;
    float limit(const Axis &axis,const float& distance){
        float ans=distance;
        if(axis.limit.max!=empty && distance > axis.limit.max)ans=axis.limit.max;
        if(axis.limit.min!=empty && distance < axis.limit.min)ans=axis.limit.min;
        return ans;
    }
    virtual float getExetndedParentFiller(const Axis &axis){
        float ans = 0;
        //蛤？父类长宽由我定？
        if((axis.extended||axis.scaleBody||axis.scaleHead||axis.scaleTail)){
            //我也不知道啊
            if(child!=nullptr)  /*去我子类那问吧*/
                ans = child->getExetndedParentFiller(axis);
            else                /*草（日语） 我没子类了*/
                throw "无法计算坐标";
        }
        else{
            //我知道
            ans = limit(axis,axis.body);
        }
        return ans + axis.head + axis.tail;//加上缝隙
    }
    virtual void calcuAxis(Axis &axis,float &begin,float &distance,float &end,
            const float& parent_begin,const float& parent_distance){
        float tmp;
        //直接穷举 通俗易懂
        if(axis.head==empty && axis.body!=empty && axis.tail==empty){
            //[   ],body,[   ]
            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
            distance = limit(axis,axis.extended ? getExetndedParentFiller(axis) : tmp);
            begin = (parent_distance - distance)/2;
            end = begin;
        }
        else if(axis.head==empty && axis.body!=empty && axis.tail!=empty){
            //[   ],body,tail
            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
            distance = limit(axis,axis.extended ? getExetndedParentFiller(axis) : tmp);
            end = axis.scaleTail ? axis.tail*parent_distance : axis.tail;
            begin = parent_distance - distance - end;
        }
        else if(axis.head != empty && axis.body!=empty && axis.tail==empty){
            //head,body,[   ]
            tmp = axis.scaleBody ? axis.body*parent_distance : axis.body;
            distance = limit(axis,axis.extended ? getExetndedParentFiller(axis) : tmp);
            begin = axis.scaleHead ? axis.head*parent_distance : axis.head;
            end = parent_distance - distance - begin;
        }
        else if(axis.head!=empty && axis.body==empty && axis.tail!=empty){
            //head,[   ],tail
            begin = axis.scaleHead ? axis.head*parent_distance : axis.head;
            end = axis.scaleTail ? axis.tail*parent_distance : axis.tail;
            float tmp = parent_distance - begin - end;
            distance = limit(axis,axis.extended ? getExetndedParentFiller(axis) : tmp);
            if(tmp!=distance) {
                begin += (tmp - distance)/2;
                end += (tmp - distance)/2;
            }
        }
    }
    void calcuRegion(Layout* container){
        child->region.setObsolete();
        calcuAxis(x,region.x,region.w,region.r,container->region.x,container->region.w);
        calcuAxis(y,region.y,region.h,region.b,container->region.y,container->region.h);
	}
#undef empty
};
const float Layout::empty = std::numeric_limits<float>::lowest();

class MutiWidget;
class Widget:public Layout
{
	friend MutiWidget;
	bool hide=false;
public:
    absRegion region;
public:

    virtual void render(HDC hdc)
    {
        Rectangle(hdc,region.x,region.y,region.x+region.w,region.y+region.h);
        if(child!=nullptr)
        {
            child->resetRegion(this);
            child->render(hdc);
        }

    }

	static Widget *Zero;
	Widget()=default;
	Widget(DOM::initializer property)
	{
		DOM::moveProperty(property, this);
	}
	//留给派生类重写
    virtual float measureChildWidth()
    {
        int val = 0;
        Widget *iter = this;
        if (iter != nullptr)
        {
            while (iter->neighbor_left != nullptr)
                iter = iter->neighbor_left;
            while (iter != nullptr)
            {
                iter->resetRegion(Zero);
                val += iter->region.w;
                iter = iter->neighbor_right;
            }
            iter = this;
            while (iter->neighbor_top != nullptr)
                iter = iter->neighbor_top;
            while (iter != nullptr)
            {
                iter->resetRegion(Zero);
                if (iter->region.w > val)
                    val = iter->region.w;
                iter = iter->neighbor_bottom;
            }
        }
        return val;
    }
    virtual float measureChildHeight()
    {
        int val = 0;
        Widget *iter = this;
        if (iter != nullptr)
        {
            while (iter->neighbor_top != nullptr)
                iter = iter->neighbor_top;
            while (iter != nullptr)
            {
                iter->resetRegion(Zero);
                val += iter->region.h;
                iter = iter->neighbor_bottom;
            }
            iter = this;
            while (iter->neighbor_left != nullptr)
                iter = iter->neighbor_left;
            while (iter != nullptr)
            {
                iter->resetRegion(Zero);
                if (iter->region.h > val)
                    val = iter->region.h;
                iter = iter->neighbor_right;
            }
        }
        return val;
    }
    virtual void resetRegion(Widget *parent)
    {
        //本布局隐藏时不计算
        if(hide)return;

        //设置了宽度、高度时，直接计算w h
        if (width != nullptr)
            region.w = *width;
        else if (scale_width != nullptr)
            region.w = parent->region.w * (*scale_width);
        else if (pending_width)
            region.w = measureChildWidth();

        //左右都没设置，只设置了宽，那就水平居中
        if (pending_left == nullptr && pending_right == nullptr && left == nullptr && scale_left == nullptr && right == nullptr && scale_right == nullptr && (width != nullptr || scale_width != nullptr || pending_width))
        {
            region.x = parent->region.x + (parent->region.w - region.w) / 2;
        }

        //设置了left、scale_left或pending_left时，计算x
        if (parent->pending_width && pending_left == nullptr && pending_right == nullptr)
        {
            region.x = parent->region.x;
        }
        else if (pending_left != nullptr)
        {
            region.x = pending_left->region.x + pending_left->region.w;
        }
        else if (left != nullptr)
        {
            region.x = parent->region.x + *left;
        }
        else if (scale_left != nullptr)
        {
            region.x = parent->region.x + parent->region.w * (*scale_left);
        }

        //设置了right、scale_right时，如果同时设置了width或scale_width（此时left无效），计算x；否则结合left计算w
        //计算pending_right pending_bottom需要region.h region.w,故放在后面计算
        if (pending_right == nullptr && pending_left == nullptr)
        {
            if (right != nullptr)
            {
                if (width != nullptr || scale_width != nullptr || pending_width)
                {
                    region.x = parent->region.x + parent->region.w - region.w - *right;
                }
                else
                {
                    region.w = parent->region.x + parent->region.w - region.x - *right;
                }
            }
            else if (scale_right != nullptr)
            {
                if (width != nullptr || scale_width != nullptr || pending_height)
                {
                    region.x = parent->region.x + parent->region.w - region.w - parent->region.w * (*scale_right);
                }
                else
                {
                    region.w = parent->region.x + parent->region.w - region.x - parent->region.w * (*scale_right);
                }
            }
        }

        if (height != nullptr)
            region.h = *height;
        else if (scale_height != nullptr)
            region.h = parent->region.h * (*scale_height);
        else if (pending_height)
            region.h = measureChildHeight();

        //上下都没设置，只设置了高，那就垂直居中
        if (pending_top == nullptr && pending_bottom == nullptr && top == nullptr && scale_top == nullptr && bottom == nullptr && scale_bottom == nullptr && (height != nullptr || scale_height != nullptr || pending_height))
        {
            region.y = parent->region.y + (parent->region.h - region.h) / 2;
        }

        //设置了top、scale_top或pending_top时，计算y
        if (parent->pending_height && pending_top == nullptr && pending_bottom == nullptr)
        {
            region.y = parent->region.y;
        }
        else if (pending_top != nullptr)
        {
            region.y = pending_top->region.y + pending_top->region.h;
        }
        if (top != nullptr)
        {
            region.y = parent->region.y + *top;
        }
        else if (scale_top != nullptr)
        {
            region.y = parent->region.y + parent->region.h * (*scale_top);
        }

        //设置了bottom、scale_bottom时，如果同时设置了height或scale_height（此时top无效），计算x；否则结合top计算h
        if (pending_bottom == nullptr && pending_top == nullptr)
        {
            if (bottom != nullptr)
            {
                if (height != nullptr || scale_height != nullptr || pending_height)
                {
                    region.y = parent->region.y + parent->region.h - region.h - *bottom;
                }
                else
                {
                    region.h = parent->region.y + parent->region.h - region.y - *bottom;
                }
            }
            else if (scale_bottom != nullptr)
            {
                if (height != nullptr || scale_height != nullptr || pending_height)
                {
                    region.y = parent->region.y + parent->region.h - region.h - parent->region.h * (*scale_bottom);
                }
                else
                {
                    region.h = parent->region.y + parent->region.h - region.y - parent->region.h * (*scale_bottom);
                }
            }
        }

        //限制尺寸在min和max之间，没有设置min max则不限制
        if (max_width != nullptr)
        {
            if (region.w > *max_width)
                region.w = *max_width;
        }
        else if (scale_max_width != nullptr)
        {
            if (region.w / parent->region.w > *scale_max_width)
                region.w = parent->region.w * (*scale_max_width);
        }

        if (min_width != nullptr)
        {
            if (region.w < *min_width)
                region.w = *min_width;
        }
        else if (scale_min_width != nullptr)
        {
            if (region.w / parent->region.w < *scale_min_width)
                region.w = parent->region.w * (*scale_min_width);
        }

        if (max_height != nullptr)
        {
            if (region.h > *max_height)
                region.h = *max_height;
        }
        else if (scale_max_height != nullptr)
        {
            if (region.h / parent->region.h > *scale_max_height)
                region.h = parent->region.h * (*scale_max_height);
        }

        if (min_height != nullptr)
        {
            if (region.h < *min_height)
                region.h = *min_height;
        }
        else if (scale_min_height != nullptr)
        {
            if (region.h / parent->region.h < *scale_min_height)
                region.h = parent->region.h * (*scale_min_height);
        }

        //调整right、bottom待定的layout 因为计算需要得知w h所以放在尺寸限制计算之后
        if (!pending_width && pending_right != nullptr)
        {
            region.x = pending_right->region.x - region.w;
        }
        if (!pending_height && pending_bottom != nullptr)
        {
            region.y = pending_bottom->region.y - region.h;
        }
    }
	enum class Edge
	{
		left,
		top,
		right,
		bottom
	};
	//void setContent(Widget* content) {
	//	if(content!=nullptr)delete content;
	//	this->display->content=content;
	//	//添加region到脏矩形
	//	//....
	//	//通知刷新
	//}
	std::list<Widget *> content;

	//好像构造函数里设置child和neighbor不是很舒服...以后看看怎么改好了
	//
	//void setChild(Layout* layout) {
	//	//if(child!=nullptr)delete child;
	//	this->child=layout;
	//	//layout->parent=this;
	//	display=child;
	//	//添加region到脏矩形
	//	//resetRegion
	//	//添加region到脏矩形
	//	//....
	//	//通知刷新
	//}
	//设置和目标layout紧挨。例如Edge::left是将本实例的左边和目标layout的右边对齐
	//同时目标layout成为本实例的pending_left
	//本实例成为目标layout的neighbor_right
	void setNeighbor(Widget* widget, Edge edge) {
		switch (edge) {
		case Edge::left:
			if(pending_left!=nullptr)delete pending_left;
			pending_left=widget;
			widget->neighbor_right=this;
			//parent= layout->parent;
			break;
		case Edge::top:
			if(pending_top!=nullptr)delete pending_top;
			pending_top=widget;
			widget->neighbor_bottom=this;
			//parent = layout->parent;
			break;
		case Edge::right:
			if(pending_right!=nullptr)delete pending_right;
			pending_right=widget;
			widget->neighbor_left=this;
			//parent = layout->parent;
			break;
		case Edge::bottom:
			if(pending_bottom!=nullptr)delete pending_bottom;
			pending_bottom=widget;
			widget->neighbor_top=this;
			//parent = layout->parent;
			break;
		}
		//添加region到脏矩形
		//resetRegion
		//添加region到脏矩形
		//....
		//通知刷新
	}

};
Widget *Widget::Zero = new Widget();

using displayCondition = std::function<bool(Size)>;

class MutiWidget : public Widget
{
	displayCondition smaller_condition, larger_condition;
	Widget *smaller,*larger,*normal;
	virtual void resetRegion(Widget *parent) override
	{
        //选择合适的layout
        Size size;
        bool flag = false;
        size.height = region.h;
        size.width = region.w;
        size.scale_height = region.h / parent->region.h;
        size.scale_width = region.w / parent->region.w;
        if (larger_condition)
        {
            if (larger_condition(size))
            {
                flag = true;
                larger->resetRegion(parent);
            }
        }
        if (smaller_condition)
        {
            if (smaller_condition(size))
            {
                flag = true;
                smaller->resetRegion(parent);
            }
        }
        if (!flag)normal->resetRegion(parent);;
	}

public:
	MutiWidget(Widget* smaller, displayCondition smaller_condition, Widget* normal,
			   Widget* larger, displayCondition larger_condition)
	{
		
		this->smaller = smaller;
		this->larger = larger;
		this->larger_condition = larger_condition;
		this->smaller_condition = smaller_condition;
	}
	MutiWidget(Widget* smaller, displayCondition smaller_condition, Widget* normal)
	{
		this->normal = normal;
		this->smaller = smaller;
		this->smaller_condition = smaller_condition;
	}
	MutiWidget(Widget* normal, Widget* larger, displayCondition larger_condition)
	{
		this->normal = normal;
		this->larger = larger;
		this->larger_condition = larger_condition;
	}

	//void setSmallerChild(Layout* layout, displayCondition condition) {
	//	if (smaller_layout != nullptr)delete smaller_layout;
	//	this->smaller_layout = layout;
	//	//if(layout!=nullptr)layout->parent=this;
	//	this->smaller_condition = condition;
	//	//添加region到脏矩形
	//	//resetRegion
	//	//添加region到脏矩形
	//	//....
	//	//通知刷新
	//}

	//void setLargerChild(Layout* layout, displayCondition condition) {
	//	if (larger_layout != nullptr)delete larger_layout;
	//	this->larger_layout = layout;
	//	//if (layout != nullptr)layout->parent=this;
	//	this->larger_condition = condition;
	//	//添加region到脏矩形
	//	//resetRegion
	//	//添加region到脏矩形
	//	//....
	//	//通知刷新
	//}
};

class ScrollField:public Widget
{
public:
    ScrollField(){}

};

class Canvas : public Widget
{

};

class Stack : public Widget
{

};

class Grid:public Widget
{
    int colm,rowm;
    struct Container
    {
        Widget* widget;
        Container(int row,int col,int spanRow,int spanCol,Widget* widget):
        row(row),col(col),spanRow(spanRow),spanCol(spanCol){
            this->widget->child=widget;
            widget->left=new int(0);
            widget->top=new int(0);
            widget->width=new int(0);
            widget->height=new int(0);
        }
        int row,col,spanRow,spanCol;
    };
    std::vector<Container*> content;
    std::vector<std::vector<int>> table;
    Container* get(int col,int row) {
        return content[table[col][row]];
    }
public:
    virtual void render(HDC hdc)override
    {
        Rectangle(hdc,region.x,region.y,region.x+region.w,region.y+region.h);
        for(auto c:content)
        {
            if(c!=nullptr)
            {
                c->widget->resetRegion(this);
                c->widget->render(hdc);
            }
        }
    }
    void resetRegion(Widget* parent)override
    {
        Widget::resetRegion(parent);
        int unitW= region.w / colm;
        int unitH= region.h / rowm;
        for(auto c:content)
        {
            Layout* d=c->widget;
            *d->left=c->col*unitW;
            *d->top=c->row*unitH;
            *d->width=c->spanCol*unitW;
            *d->height=c->spanRow*unitH;
        }
    }

    Grid(int columns,int rows,DOM::initializer property)
    {
        colm=columns;
        rowm=rows;
        DOM::moveProperty(property,this);

        table.resize(rows);
        for (int i = 0; i < rows; i++)
            table[i].resize(columns);
    }

    void setChild(int col,int row,int spanCol,int spanRow,Widget* widget)
    {
        if(col>-1 && col < colm && row > -1 && row < rowm &&
            spanCol>-1 && spanCol < colm && spanRow > -1 && spanRow < row)
        {
            content.push_back(new Container(col,row,spanRow,spanCol,widget));
            int index=content.size();
            for(int y=col;y<col+spanCol;y++)
                for(int x=row;x<row+spanRow;x++)
                {
                    delete get(x,y)->widget;
                    delete get(x,y);
                    content[table[x][y]]=nullptr;
                    table[y][x]=index;
                }

        }
    }

    void setChild(int col,int row,Widget* widget)
    {
        setChild(col,row,1,1,widget);
    }

};

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
