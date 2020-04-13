#ifndef KAT_LAYOUT
#define KAT_LAYOUT
#include<functional>
#include<list>
#include<typeindex>
#include<memory>
#include<string>

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

class Layout;
class layoutProperties {
public:
	friend Layout;
	//表示上下左右紧贴的对象 nullptr时相应距离变量无效
	int* left = nullptr, * top = nullptr, * right = nullptr, * bottom = nullptr;
	//这四个变量供计算使用
	Layout* pending_top = nullptr, * pending_left = nullptr, * pending_right = nullptr, * pending_bottom = nullptr;
	//这四个变量供绘图时使用，用来找到四个方向紧贴自己的邻居。neighbor_xxx与pending_xxx形成双向链表
	//同时也避免计算时两个紧贴的layout形成“空岛”（A告知在B右边、B告知在A左边）造成无法计算坐标的现象
	Layout* neighbor_top = nullptr, * neighbor_left = nullptr, * neighbor_right = nullptr, * neighbor_bottom = nullptr;
	//表示大小由子对象决定，pending为true时限制行为（max、min）依然有效
	bool pending_width = false, pending_height = false;
	int* width = nullptr, * max_width = nullptr, * min_width = nullptr;
	int* height = nullptr, * max_height = nullptr, * min_height = nullptr;

	float* scale_left = nullptr, * scale_top = nullptr, * scale_right = nullptr, * scale_bottom = nullptr;
	float* scale_width = nullptr, * scale_max_width = nullptr, * scale_min_width = nullptr;
	float* scale_height = nullptr, * scale_max_height = nullptr, * scale_min_height = nullptr;
	//Layout* display = nullptr;/*,*parent=nullptr;*/ //考虑到多布局时一个布局可能同时是几个布局的子对象（这几个布局不会同时出现）故移除此变量
	Layout* child = nullptr;
	layoutProperties operator+(const layoutProperties obj) {
#define FILL(ptr) if(ptr==nullptr)ptr=obj.ptr
		FILL(left);			FILL(top);				FILL(right);			FILL(bottom); 
		FILL(pending_top);	FILL(pending_left);		FILL(pending_right);	FILL(pending_bottom); 
		FILL(neighbor_top); FILL(neighbor_left);	FILL(neighbor_right);	FILL(neighbor_bottom); 
		FILL(scale_left);	FILL(scale_top);		FILL(scale_right);		FILL(scale_bottom);
		FILL(width);		FILL(max_width);		FILL(min_width);		
		FILL(height);		FILL(min_height);		FILL(max_height);
		FILL(scale_width);	FILL(scale_max_width);	FILL(scale_min_width);
		FILL(scale_height);	FILL(scale_max_height); FILL(scale_min_height); 
		FILL(child);
#undef FILL(ptr)
		if(obj.pending_width)pending_width=true;
		if(obj.pending_height)pending_height=true;
		return *this;
	}
};

//layoutProperties left(int px) {
//	layoutProperties tmp;
//	tmp.left=new int(px);
//	return tmp;
//}
//layoutProperties top(int px) {
//	layoutProperties tmp;
//	tmp.top=new int(px);
//	return tmp;
//}
//layoutProperties right(int px) {
//	layoutProperties tmp;
//	tmp.right=new int(px);
//	return tmp;
//}
//layoutProperties bottom(int px) {
//	layoutProperties tmp;
//	tmp.bottom=new int(px);
//	return tmp;
//}
//layoutProperties limitMinWidth(int px) {
//	layoutProperties tmp;
//	tmp.min_width=new int(px);
//	return tmp;
//}
//layoutProperties limitMinWidth(float scale) {
//	layoutProperties tmp;
//	tmp.scale_min_width=new float(scale);
//	return tmp;
//}
//layoutProperties limitMaxWidth(int px) {
//	layoutProperties tmp;
//	tmp.max_width=new int(px);
//	return tmp;
//}
//layoutProperties limitMaxWidth(float scale) {
//	layoutProperties tmp;
//	tmp.scale_max_width = new float(scale);
//	return tmp;
//}
//
//layoutProperties limitMinHeight(int px) {
//	layoutProperties tmp;
//	tmp.min_height = new int(px);
//	return tmp;
//}
//layoutProperties limitMinHeight(float scale) {
//	layoutProperties tmp;
//	tmp.scale_min_height = new float(scale);
//	return tmp;
//}
//layoutProperties limitMaxHeight(int px) {
//	layoutProperties tmp;
//	tmp.max_height = new int(px);
//	return tmp;
//}
//layoutProperties limitMaxHeight(float scale) {
//	layoutProperties tmp;
//	tmp.scale_max_height = new float(scale);
//	return tmp;
//}

class Layout{
public:
	static Layout* Zero;
public:
	layoutProperties* display=nullptr;
	layoutProperties data;
	absRegion region;
	Layout():Layout()
	Layout(layoutProperties properties) {
		data=properties;
		display=&data;
	}
	//留给派生类重写
	virtual float measureChildWidth(){
		int val=0;
		Layout *iter=this;
		if (iter != nullptr)
		{
			while (iter->display->neighbor_left != nullptr) 
				iter=iter->display->neighbor_left;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				val+=iter->region.w;
				iter=iter->display->neighbor_right;
			}
			iter = this;
			while (iter->display->neighbor_top != nullptr)
				iter = iter->display->neighbor_top;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				if(iter->region.w>val)val=iter->region.w;
				iter = iter->display->neighbor_bottom;
			}
		}
		return val;
	}
	virtual float measureChildHeight(){
		int val = 0;
		Layout* iter = this;
		if (iter != nullptr)
		{
			while (iter->display->neighbor_top != nullptr)
				iter = iter->display->neighbor_top;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				val += iter->region.h;
				iter = iter->display->neighbor_bottom;
			}
			iter = this;
			while (iter->display->neighbor_left != nullptr)
				iter = iter->display->neighbor_left;
			while (iter != nullptr)
			{
				iter->resetRegion(Zero);
				if(iter->region.h>val)val=iter->region.h;
				iter = iter->display->neighbor_right;
			}
		}
		return val;
	}
	virtual void resetRegion(Layout* parent) {
		//本布局隐藏时不计算
		if(display==nullptr)return;

		//设置了宽度、高度时，直接计算w h
		if (display->width != nullptr)region.w = *display->width;
		else if (display->scale_width != nullptr)region.w = parent->region.w * (*display->scale_width);
		else if (display->pending_width) region.w = measureChildWidth();
		 

		//左右都没设置，只设置了宽，那就水平居中
		if (display->pending_left == nullptr && display->pending_right == nullptr
			&& display->left == nullptr && display->scale_left == nullptr && display->right == nullptr && display->scale_right == nullptr
			&& (display->width != nullptr || display->scale_width != nullptr || display->pending_width)) {
			region.x = parent->region.x + (parent->region.w - region.w) / 2;
		}

		//设置了left、scale_left或pending_left时，计算x
		if (parent->display->pending_width && display->pending_left == nullptr && display->pending_right == nullptr) {
			region.x = parent->region.x;
		}
		else if (display->pending_left != nullptr) {
			region.x = display->pending_left->region.x + display->pending_left->region.w;
		}
		else if (display->left != nullptr) {
			region.x = parent->region.x + *display->left;
		}
		else if (display->scale_left != nullptr) {
			region.x = parent->region.x + parent->region.w * (*display->scale_left);
		}

		//设置了right、scale_right时，如果同时设置了width或scale_width（此时left无效），计算x；否则结合left计算w
		//计算pending_right pending_bottom需要region.h region.w,故放在后面计算
		if (display->pending_right == nullptr && display->pending_left == nullptr) {
			if (display->right != nullptr) {
				if (display->width != nullptr || display->scale_width != nullptr || display->pending_width) {
					region.x = parent->region.x + parent->region.w - region.w - *display->right;
				}
				else {
					region.w = parent->region.x + parent->region.w - region.x - *display->right;
				}
			}
			else if (display->scale_right != nullptr) {
				if (display->width != nullptr || display->scale_width != nullptr || display->pending_height) {
					region.x = parent->region.x + parent->region.w - region.w - parent->region.w * (*display->scale_right);
				}
				else {
					region.w = parent->region.x + parent->region.w - region.x - parent->region.w * (*display->scale_right);
				}
			}
		}


		if (display->height != nullptr)region.h = *display->height;
		else if (display->scale_height != nullptr)region.h = parent->region.h * (*display->scale_height);
		else if (display->pending_height) region.h = measureChildHeight();


		//上下都没设置，只设置了高，那就垂直居中
		if (display->pending_top == nullptr && display->pending_bottom == nullptr
			&& display->top == nullptr && display->scale_top == nullptr && display->bottom == nullptr && display->scale_bottom == nullptr
			&& (display->height != nullptr || display->scale_height != nullptr || display->pending_height)) {
			region.y = parent->region.y + (parent->region.h - region.h) / 2;
		}

		//设置了top、scale_top或pending_top时，计算y
		if (parent->display->pending_height && display->pending_top == nullptr && display->pending_bottom == nullptr) {
			region.y = parent->region.y;
		}
		else if (display->pending_top != nullptr) {
			region.y = display->pending_top->region.y + display->pending_top->region.h;
		}
		if (display->top != nullptr) {
			region.y = parent->region.y + *display->top;
		}
		else if (display->scale_top != nullptr) {
			region.y = parent->region.y + parent->region.h * (*display->scale_top);
		}

		//设置了bottom、scale_bottom时，如果同时设置了height或scale_height（此时top无效），计算x；否则结合top计算h
		if (display->pending_bottom == nullptr && display->pending_top == nullptr) {
			if (display->bottom != nullptr) {
				if (display->height != nullptr || display->scale_height != nullptr || display->pending_height) {
					region.y = parent->region.y + parent->region.h - region.h - *display->bottom;
				}
				else {
					region.h = parent->region.y + parent->region.h - region.y - *display->bottom;
				}
			}
			else if (display->scale_bottom != nullptr) {
				if (display->height != nullptr || display->scale_height != nullptr || display->pending_height) {
					region.y = parent->region.y + parent->region.h - region.h - parent->region.h * (*display->scale_bottom);
				}
				else {
					region.h = parent->region.y + parent->region.h - region.y - parent->region.h * (*display->scale_bottom);
				}
			}
		}


		//限制尺寸在min和max之间，没有设置min max则不限制
		if (display->max_width != nullptr) {
			if (region.w > * display->max_width) region.w = *display->max_width;
		}
		else if (display->scale_max_width != nullptr) {
			if (region.w / parent->region.w > * display->scale_max_width)region.w = parent->region.w * (*display->scale_max_width);
		}

		if (display->min_width != nullptr) {
			if (region.w < *display->min_width) region.w = *display->min_width;
		}
		else if (display->scale_min_width != nullptr) {
			if (region.w / parent->region.w < *display->scale_min_width)region.w = parent->region.w * (*display->scale_min_width);
		}

		if (display->max_height != nullptr) {
			if (region.h > * display->max_height) region.h = *display->max_height;
		}
		else if (display->scale_max_height != nullptr) {
			if (region.h / parent->region.h > * display->scale_max_height)region.h = parent->region.h * (*display->scale_max_height);
		}

		if (display->min_height != nullptr) {
			if (region.h < *display->min_height) region.h = *display->min_height;
		}
		else if (display->scale_min_height != nullptr) {
			if (region.h / parent->region.h < *display->scale_min_height)region.h = parent->region.h * (*display->scale_min_height);
		}

		//调整right、bottom待定的layout 因为计算需要得知w h所以放在尺寸限制计算之后
		if (!display->pending_width && display->pending_right != nullptr) {
			region.x = display->pending_right->region.x - region.w;
		}
		if (!display->pending_height && display->pending_bottom != nullptr) {
			region.y = display->pending_bottom->region.y - region.h;
		}
	}
public:
	enum class Edge{left,top,right,bottom};
	//void setContent(Widget* content) {
	//	if(content!=nullptr)delete content;
	//	this->display->content=content;
	//	//添加region到脏矩形
	//	//....
	//	//通知刷新
	//}
	std::list<Widget*> content;

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
	//void setNeighbor(Layout* layout, Edge edge) {
	//	switch (edge) {
	//	case Edge::left:
	//		if(pending_left!=nullptr)delete pending_left;
	//		pending_left=layout;
	//		layout->neighbor_right=this;
	//		//parent= layout->parent;
	//		break;
	//	case Edge::top:
	//		if(pending_top!=nullptr)delete pending_top;
	//		pending_top=layout;
	//		layout->neighbor_bottom=this;
	//		//parent = layout->parent;
	//		break;
	//	case Edge::right:
	//		if(pending_right!=nullptr)delete pending_right;
	//		pending_right=layout;
	//		layout->neighbor_left=this;
	//		//parent = layout->parent;
	//		break;
	//	case Edge::bottom:
	//		if(pending_bottom!=nullptr)delete pending_bottom;
	//		pending_bottom=layout;
	//		layout->neighbor_top=this;
	//		//parent = layout->parent;
	//		break;
	//	}
	//	//添加region到脏矩形
	//	//resetRegion
	//	//添加region到脏矩形
	//	//....
	//	//通知刷新
	//}
	
};
Layout* Layout::Zero = new Layout(layoutProperties());

using displayCondition=std::function<bool(Size)>;

class MutiLayout:public Layout {
	displayCondition smaller_condition,larger_condition;
	layoutProperties smaller_layout,larger_layout;
	virtual void resetRegion(Layout* parent)override {
		//选择合适的layout
		Size size;
		bool flag = false;
		size.height = region.h;
		size.width = region.w;
		size.scale_height = region.h / parent->region.h;
		size.scale_width = region.w / parent->region.w;
		if (larger_condition) {
			if (larger_condition(size)) {
				display = &larger_layout;
				flag = true;
			}
		}
		if (smaller_condition) {
			if (smaller_condition(size)) {
				display = &smaller_layout;
				flag = true;
			}
		}
		if (!flag)display = &data;

		Layout::resetRegion(parent);
	}
public:
	MutiLayout(layoutProperties smaller, displayCondition smaller_condition,layoutProperties normal,
									   layoutProperties larger, displayCondition larger_condition):Layout(normal){
		this->data=normal;
		this->smaller_layout=smaller;
		this->larger_layout=larger;
		this->larger_condition=larger_condition;
		this->smaller_condition=smaller_condition;
	}
	MutiLayout(layoutProperties smaller, displayCondition smaller_condition,layoutProperties normal) :Layout(normal) {
		this->data = normal;
		this->smaller_layout = smaller;
		this->smaller_condition = smaller_condition;
	}
	MutiLayout(layoutProperties normal,layoutProperties larger, displayCondition larger_condition) :Layout(normal) {
		this->data = normal;
		this->larger_layout = larger;
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

class Canvas :public Layout {

};

class Stack:public Layout {

};

class Grid {

};

namespace DOM {
	struct Any
	{
		Any(void) : m_tpIndex(std::type_index(typeid(void))) {}
		Any(const Any& that) : m_ptr(that.Clone()), m_tpIndex(that.m_tpIndex) {}
		Any(Any&& that) : m_ptr(std::move(that.m_ptr)), m_tpIndex(that.m_tpIndex) {}

		//创建智能指针时，对于一般的类型，通过std::decay来移除引用和cv符，从而获取原始类型
		template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type> Any(U&& value) : m_ptr(new Derived < typename std::decay<U>::type>(forward<U>(value))),
			m_tpIndex(type_index(typeid(typename std::decay<U>::type))) {}

		bool IsNull() const { return !bool(m_ptr); }

		template<class U> bool Is() const
		{
			return m_tpIndex == type_index(typeid(U));
		}

		//将Any转换为实际的类型
		template<class U>
		U& AnyCast()
		{
			if (!Is<U>())
			{
				cout << "can not cast " << typeid(U).name() << " to " << m_tpIndex.name() << endl;
				throw bad_cast();
			}

			auto derived = dynamic_cast<Derived<U>*> (m_ptr.get());
			return derived->m_value;
		}

		Any& operator=(const Any& a)
		{
			if (m_ptr == a.m_ptr)
				return *this;

			m_ptr = a.Clone();
			m_tpIndex = a.m_tpIndex;
			return *this;
		}

	private:
		struct Base;
		typedef std::unique_ptr<Base> BasePtr;

		struct Base
		{
			virtual ~Base() {}
			virtual BasePtr Clone() const = 0;
		};

		template<typename T>
		struct Derived : Base
		{
			template<typename U>
			Derived(U&& value) : m_value(forward<U>(value)) { }

			BasePtr Clone() const
			{
				return BasePtr(new Derived<T>(m_value));
			}

			T m_value;
		};

		BasePtr Clone() const
		{
			if (m_ptr != nullptr)
				return m_ptr->Clone();

			return nullptr;
		}

		BasePtr m_ptr;
		std::type_index m_tpIndex;
	};
	
	Item left(int px) {

	}
	Item top(int px) {

	}
	Item right(int px) {

	}
	Item bottom(int px) {

	}
	Item height(int px) {

	}
	Item width(int px) {

	}
	Item child(Layout* layout) {

	}
}
#endif
