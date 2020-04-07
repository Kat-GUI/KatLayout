#ifndef KAT_LAYOUT
#define KAT_LAYOUT
#include<functional>
class Widget{
public:
};
struct absRegion {
	float x=0,y=0,w=0,h=0;
};
struct Size {
	int height,width;
	float scale_height,scale_width;
};
class Layout{
public:
	static Layout* Zero;
public:
	//表示上下左右紧贴的对象 nullptr时相应距离变量无效
	int *left=nullptr,*top=nullptr,*right=nullptr,*bottom=nullptr;
	//这四个变量供计算使用
	Layout *pending_top=nullptr,*pending_left=nullptr,*pending_right=nullptr,*pending_bottom=nullptr;
	//这四个变量供绘图时使用，用来找到四个方向紧贴自己的邻居。neighbor_xxx与pending_xxx形成双向链表
	//同时也避免计算时两个紧贴的layout形成“空岛”（A告知在B右边、B告知在A左边）造成无法计算坐标的现象
	Layout *neighbor_top=nullptr,*neighbor_left=nullptr,*neighbor_right=nullptr,*neighbor_bottom=nullptr;
	//表示大小由子对象决定，pending为true时限制行为（max、min）依然有效
	bool pending_width=false,pending_height=false;
	int *width=nullptr,*max_width=nullptr,*min_width=nullptr;
	int *height=nullptr,*max_height=nullptr,*min_height=nullptr;

	float *scale_left=nullptr,*scale_top=nullptr,*scale_right=nullptr,*scale_bottom=nullptr;
	float *scale_width=nullptr,*scale_max_width=nullptr,*scale_min_width=nullptr;
	float *scale_height=nullptr,*scale_max_height=nullptr,*scale_min_height=nullptr;
	Widget* content;
	Layout *display=nullptr;/*,*parent=nullptr;*/ //考虑到多布局时一个布局可能同时是几个布局的子对象（这几个布局不会同时出现）故移除此变量
	Layout *min_child=nullptr,*max_child=nullptr,*child=nullptr;
	using Condition = std::function<bool(Size)>;
	Condition min_condition,max_condition;
	absRegion region;
	//留给派生类重写
	virtual float measureChildWidth(){
		int val=0;
		Layout *iter=display;
		if (iter != nullptr)
		{
			while (iter->neighbor_left != nullptr) 
				iter=iter->neighbor_left;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				val+=iter->region.w;
				iter=iter->neighbor_right;
			}
			iter = display;
			while (iter->neighbor_top != nullptr)
				iter = iter->neighbor_top;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				if(iter->region.w>val)val=iter->region.w;
				iter = iter->neighbor_bottom;
			}
		}
		return val;
	}
	virtual float measureChildHeight(){
		int val = 0;
		Layout* iter = display;
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
			iter = display;
			while (iter->neighbor_left != nullptr)
				iter = iter->neighbor_left;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				if(iter->region.h>val)val=iter->region.h;
				iter = iter->neighbor_right;
			}
		}
		return val;
	}
	void resetRegion(Layout* parent) {
		//设置了宽度、高度时，直接计算w h
		if (width != nullptr)region.w = *width;
		else if (scale_width != nullptr)region.w = parent->region.w * (*scale_width);
		else if (pending_width) region.w = measureChildWidth();
		 

		//左右都没设置，只设置了宽，那就水平居中
		if (pending_left == nullptr && pending_right == nullptr
			&& left == nullptr && scale_left == nullptr && right == nullptr && scale_right == nullptr
			&& (width != nullptr || scale_width != nullptr || pending_width)) {
			region.x = parent->region.x + (parent->region.w - region.w) / 2;
		}

		//设置了left、scale_left或pending_left时，计算x
		if (parent->pending_width && pending_left == nullptr && pending_right == nullptr) {
			region.x = parent->region.x;
		}
		else if (pending_left != nullptr) {
			region.x = pending_left->region.x + pending_left->region.w;
		}
		else if (left != nullptr) {
			region.x = parent->region.x + *left;
		}
		else if (scale_left != nullptr) {
			region.x = parent->region.x + parent->region.w * (*scale_left);
		}

		//设置了right、scale_right时，如果同时设置了width或scale_width（此时left无效），计算x；否则结合left计算w
		//计算pending_right pending_bottom需要region.h region.w,故放在后面计算
		if (pending_right == nullptr && pending_left == nullptr) {
			if (right != nullptr) {
				if (width != nullptr || scale_width != nullptr || pending_width) {
					region.x = parent->region.x + parent->region.w - region.w - *right;
				}
				else {
					region.w = parent->region.x + parent->region.w - region.x - *right;
				}
			}
			else if (scale_right != nullptr) {
				if (width != nullptr || scale_width != nullptr || pending_height) {
					region.x = parent->region.x + parent->region.w - region.w - parent->region.w * (*scale_right);
				}
				else {
					region.w = parent->region.x + parent->region.w - region.x - parent->region.w * (*scale_right);
				}
			}
		}


		if (height != nullptr)region.h = *height;
		else if (scale_height != nullptr)region.h = parent->region.h * (*scale_height);
		else if (pending_height) region.h = measureChildHeight();


		//上下都没设置，只设置了高，那就垂直居中
		if (pending_top == nullptr && pending_bottom == nullptr
			&& top == nullptr && scale_top == nullptr && bottom == nullptr && scale_bottom == nullptr
			&& (height != nullptr || scale_height != nullptr || pending_height)) {
			region.y = parent->region.y + (parent->region.h - region.h) / 2;
		}

		//设置了top、scale_top或pending_top时，计算y
		if (parent->pending_height && pending_top == nullptr && pending_bottom == nullptr) {
			region.y = parent->region.y;
		}
		else if (pending_top != nullptr) {
			region.y = pending_top->region.y + pending_top->region.h;
		}
		if (top != nullptr) {
			region.y = parent->region.y + *top;
		}
		else if (scale_top != nullptr) {
			region.y = parent->region.y + parent->region.h * (*scale_top);
		}

		//设置了bottom、scale_bottom时，如果同时设置了height或scale_height（此时top无效），计算x；否则结合top计算h
		if (pending_bottom == nullptr && pending_top == nullptr) {
			if (bottom != nullptr) {
				if (height != nullptr || scale_height != nullptr || pending_height) {
					region.y = parent->region.y + parent->region.h - region.h - *bottom;
				}
				else {
					region.h = parent->region.y + parent->region.h - region.y - *bottom;
				}
			}
			else if (scale_bottom != nullptr) {
				if (height != nullptr || scale_height != nullptr || pending_height) {
					region.y = parent->region.y + parent->region.h - region.h - parent->region.h * (*scale_bottom);
				}
				else {
					region.h = parent->region.y + parent->region.h - region.y - parent->region.h * (*scale_bottom);
				}
			}
		}


		//限制尺寸在min和max之间，没有设置min max则不限制
		if (max_width != nullptr) {
			if (region.w > * max_width) region.w = *max_width;
		}
		else if (scale_max_width != nullptr) {
			if (region.w / parent->region.w > * scale_max_width)region.w = parent->region.w * (*scale_max_width);
		}

		if (min_width != nullptr) {
			if (region.w < *min_width) region.w = *min_width;
		}
		else if (scale_min_width != nullptr) {
			if (region.w / parent->region.w < *scale_min_width)region.w = parent->region.w * (*scale_min_width);
		}

		if (max_height != nullptr) {
			if (region.h > * max_height) region.h = *max_height;
		}
		else if (scale_max_height != nullptr) {
			if (region.h / parent->region.h > * scale_max_height)region.h = parent->region.h * (*scale_max_height);
		}

		if (min_height != nullptr) {
			if (region.h < *min_height) region.h = *min_height;
		}
		else if (scale_min_height != nullptr) {
			if (region.h / parent->region.h < *scale_min_height)region.h = parent->region.h * (*scale_min_height);
		}

		//调整right、bottom待定的layout 因为计算需要得知w h所以放在尺寸限制计算之后
		if (!pending_width && pending_right != nullptr) {
			region.x = pending_right->region.x - region.w;
		}
		if (!pending_height && pending_bottom != nullptr) {
			region.y = pending_bottom->region.y - region.h;
		}

		//选择合适的子layout
		Size size;
		bool flag = false;
		size.height = region.h;
		size.width = region.w;
		size.scale_height = region.h / parent->region.h;
		size.scale_width = region.w / parent->region.w;
		if (max_condition) {
			if (max_condition(size)) {
				display = max_child;
				flag = true;
			}
		}
		if (min_condition) {
			if (min_condition(size)) {
				display = min_child;
				flag = true;
			}
		}
		if (!flag)display = child;
	}
public:
	enum class Edge{left,top,right,bottom};
	void setContent(Widget* content) {
		if(content!=nullptr)delete content;
		this->content=content;
		//添加region到脏矩形
		//....
		//通知刷新
	}
	void setChild(Layout* layout) {
		//if(child!=nullptr)delete child;
		this->child=layout;
		//layout->parent=this;
		display=child;
		//添加region到脏矩形
		//resetRegion
		//添加region到脏矩形
		//....
		//通知刷新
	}
	void setSmallerChild(Layout* layout, Condition condition) {
		if(min_child!=nullptr)delete min_child;
		this->min_child=layout;
		//if(layout!=nullptr)layout->parent=this;
		this->min_condition=condition;
		//添加region到脏矩形
		//resetRegion
		//添加region到脏矩形
		//....
		//通知刷新
	}
	void setLargerChild(Layout* layout, Condition condition) {
		if(max_child!=nullptr)delete max_child;
		this->max_child=layout;
		//if (layout != nullptr)layout->parent=this;
		this->max_condition=condition;
		//添加region到脏矩形
		//resetRegion
		//添加region到脏矩形
		//....
		//通知刷新
	}
	//设置和目标layout紧挨。例如Edge::left是将本实例的左边和目标layout的右边对齐
	//同时目标layout成为本实例的pending_left
	//本实例成为目标layout的neighbor_right
	void setNeighbor(Layout* layout, Edge edge) {
		switch (edge) {
		case Edge::left:
			if(pending_left!=nullptr)delete pending_left;
			pending_left=layout;
			layout->neighbor_right=this;
			//parent= layout->parent;
			break;
		case Edge::top:
			if(pending_top!=nullptr)delete pending_top;
			pending_top=layout;
			layout->neighbor_bottom=this;
			//parent = layout->parent;
			break;
		case Edge::right:
			if(pending_right!=nullptr)delete pending_right;
			pending_right=layout;
			layout->neighbor_left=this;
			//parent = layout->parent;
			break;
		case Edge::bottom:
			if(pending_bottom!=nullptr)delete pending_bottom;
			pending_bottom=layout;
			layout->neighbor_top=this;
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
Layout* Layout::Zero = new Layout();

class Canvas {

};

class Stack:public Layout {

};

class Grid {

};
#endif
