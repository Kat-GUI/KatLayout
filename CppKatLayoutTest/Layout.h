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
	//��ʾ�������ҽ����Ķ��� nullptrʱ��Ӧ���������Ч
	int* left = nullptr, * top = nullptr, * right = nullptr, * bottom = nullptr;
	//���ĸ�����������ʹ��
	Layout* pending_top = nullptr, * pending_left = nullptr, * pending_right = nullptr, * pending_bottom = nullptr;
	//���ĸ���������ͼʱʹ�ã������ҵ��ĸ���������Լ����ھӡ�neighbor_xxx��pending_xxx�γ�˫������
	//ͬʱҲ�������ʱ����������layout�γɡ��յ�����A��֪��B�ұߡ�B��֪��A��ߣ�����޷��������������
	Layout* neighbor_top = nullptr, * neighbor_left = nullptr, * neighbor_right = nullptr, * neighbor_bottom = nullptr;
	//��ʾ��С���Ӷ��������pendingΪtrueʱ������Ϊ��max��min����Ȼ��Ч
	bool pending_width = false, pending_height = false;
	int* width = nullptr, * max_width = nullptr, * min_width = nullptr;
	int* height = nullptr, * max_height = nullptr, * min_height = nullptr;

	float* scale_left = nullptr, * scale_top = nullptr, * scale_right = nullptr, * scale_bottom = nullptr;
	float* scale_width = nullptr, * scale_max_width = nullptr, * scale_min_width = nullptr;
	float* scale_height = nullptr, * scale_max_height = nullptr, * scale_min_height = nullptr;
	//Layout* display = nullptr;/*,*parent=nullptr;*/ //���ǵ��಼��ʱһ�����ֿ���ͬʱ�Ǽ������ֵ��Ӷ����⼸�����ֲ���ͬʱ���֣����Ƴ��˱���
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
	//������������д
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
		//����������ʱ������
		if(display==nullptr)return;

		//�����˿�ȡ��߶�ʱ��ֱ�Ӽ���w h
		if (display->width != nullptr)region.w = *display->width;
		else if (display->scale_width != nullptr)region.w = parent->region.w * (*display->scale_width);
		else if (display->pending_width) region.w = measureChildWidth();
		 

		//���Ҷ�û���ã�ֻ�����˿��Ǿ�ˮƽ����
		if (display->pending_left == nullptr && display->pending_right == nullptr
			&& display->left == nullptr && display->scale_left == nullptr && display->right == nullptr && display->scale_right == nullptr
			&& (display->width != nullptr || display->scale_width != nullptr || display->pending_width)) {
			region.x = parent->region.x + (parent->region.w - region.w) / 2;
		}

		//������left��scale_left��pending_leftʱ������x
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

		//������right��scale_rightʱ�����ͬʱ������width��scale_width����ʱleft��Ч��������x��������left����w
		//����pending_right pending_bottom��Ҫregion.h region.w,�ʷ��ں������
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


		//���¶�û���ã�ֻ�����˸ߣ��Ǿʹ�ֱ����
		if (display->pending_top == nullptr && display->pending_bottom == nullptr
			&& display->top == nullptr && display->scale_top == nullptr && display->bottom == nullptr && display->scale_bottom == nullptr
			&& (display->height != nullptr || display->scale_height != nullptr || display->pending_height)) {
			region.y = parent->region.y + (parent->region.h - region.h) / 2;
		}

		//������top��scale_top��pending_topʱ������y
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

		//������bottom��scale_bottomʱ�����ͬʱ������height��scale_height����ʱtop��Ч��������x��������top����h
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


		//���Ƴߴ���min��max֮�䣬û������min max������
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

		//����right��bottom������layout ��Ϊ������Ҫ��֪w h���Է��ڳߴ����Ƽ���֮��
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
	//	//���region�������
	//	//....
	//	//֪ͨˢ��
	//}
	std::list<Widget*> content;

	//�����캯��������child��neighbor���Ǻ����...�Ժ󿴿���ô�ĺ���
	//
	//void setChild(Layout* layout) {
	//	//if(child!=nullptr)delete child;
	//	this->child=layout;
	//	//layout->parent=this;
	//	display=child;
	//	//���region�������
	//	//resetRegion
	//	//���region�������
	//	//....
	//	//֪ͨˢ��
	//}
	//���ú�Ŀ��layout����������Edge::left�ǽ���ʵ������ߺ�Ŀ��layout���ұ߶���
	//ͬʱĿ��layout��Ϊ��ʵ����pending_left
	//��ʵ����ΪĿ��layout��neighbor_right
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
	//	//���region�������
	//	//resetRegion
	//	//���region�������
	//	//....
	//	//֪ͨˢ��
	//}
	
};
Layout* Layout::Zero = new Layout(layoutProperties());

using displayCondition=std::function<bool(Size)>;

class MutiLayout:public Layout {
	displayCondition smaller_condition,larger_condition;
	layoutProperties smaller_layout,larger_layout;
	virtual void resetRegion(Layout* parent)override {
		//ѡ����ʵ�layout
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
	//	//���region�������
	//	//resetRegion
	//	//���region�������
	//	//....
	//	//֪ͨˢ��
	//}

	//void setLargerChild(Layout* layout, displayCondition condition) {
	//	if (larger_layout != nullptr)delete larger_layout;
	//	this->larger_layout = layout;
	//	//if (layout != nullptr)layout->parent=this;
	//	this->larger_condition = condition;
	//	//���region�������
	//	//resetRegion
	//	//���region�������
	//	//....
	//	//֪ͨˢ��
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

		//��������ָ��ʱ������һ������ͣ�ͨ��std::decay���Ƴ����ú�cv�����Ӷ���ȡԭʼ����
		template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, Any>::value, U>::type> Any(U&& value) : m_ptr(new Derived < typename std::decay<U>::type>(forward<U>(value))),
			m_tpIndex(type_index(typeid(typename std::decay<U>::type))) {}

		bool IsNull() const { return !bool(m_ptr); }

		template<class U> bool Is() const
		{
			return m_tpIndex == type_index(typeid(U));
		}

		//��Anyת��Ϊʵ�ʵ�����
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
