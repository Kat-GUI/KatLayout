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
	//��ʾ�������ҽ����Ķ��� nullptrʱ��Ӧ���������Ч
	int *left=nullptr,*top=nullptr,*right=nullptr,*bottom=nullptr;
	//���ĸ�����������ʹ��
	Layout *pending_top=nullptr,*pending_left=nullptr,*pending_right=nullptr,*pending_bottom=nullptr;
	//���ĸ���������ͼʱʹ�ã������ҵ��ĸ���������Լ����ھӡ�neighbor_xxx��pending_xxx�γ�˫������
	//ͬʱҲ�������ʱ����������layout�γɡ��յ�����A��֪��B�ұߡ�B��֪��A��ߣ�����޷��������������
	Layout *neighbor_top=nullptr,*neighbor_left=nullptr,*neighbor_right=nullptr,*neighbor_bottom=nullptr;
	//��ʾ��С���Ӷ��������pendingΪtrueʱ������Ϊ��max��min����Ȼ��Ч
	bool pending_width=false,pending_height=false;
	int *width=nullptr,*max_width=nullptr,*min_width=nullptr;
	int *height=nullptr,*max_height=nullptr,*min_height=nullptr;

	float *scale_left=nullptr,*scale_top=nullptr,*scale_right=nullptr,*scale_bottom=nullptr;
	float *scale_width=nullptr,*scale_max_width=nullptr,*scale_min_width=nullptr;
	float *scale_height=nullptr,*scale_max_height=nullptr,*scale_min_height=nullptr;
	Widget* content;
	Layout *display=nullptr,*parent=nullptr;
	Layout *min_child=nullptr,*max_child=nullptr,*child=nullptr;
	using Condition = std::function<bool(Size)>;
	Condition min_condition,max_condition;
	absRegion region;
	//������������д
	virtual float measureChildWidth(){
		absRegion tmp;
		tmp.x=tmp.y=tmp.w=tmp.h=0;
		if(display!=nullptr)display->resetRegion(tmp);//�˴�������Ϊ��ȡ��w
		return display==nullptr?0:display->region.w;
	}
	virtual float measureChildHeight(){
		absRegion tmp;
		tmp.x = tmp.y = tmp.w = tmp.h = 0;
		if(display != nullptr)display->resetRegion(tmp);//�˴�������Ϊ��ȡ��h
		return display==nullptr?0:display->region.h;
	}
	void resetRegion(absRegion parent_region) {
		//�����˿�ȡ��߶�ʱ��ֱ�Ӽ���w h
		if (width != nullptr)region.w = *width;
		else if (scale_width != nullptr)region.w = parent_region.w * (*scale_width);
		else if (pending_width) region.w = measureChildWidth();

		if (height != nullptr)region.h = *height;
		else if (scale_height != nullptr)region.h = parent_region.h * (*scale_height);
		if (pending_height) region.h = measureChildHeight();

		//���Ҷ�û���ã�ֻ�����˿��Ǿ�ˮƽ����
		if (pending_left == nullptr && pending_right == nullptr 
			&& left == nullptr && scale_left == nullptr && right == nullptr && scale_right == nullptr 
			&& (width!=nullptr||scale_width!=nullptr)) {
			region.x = parent_region.x + (parent_region.w - region.w) / 2;
		}
		//���¶�û���ã�ֻ�����˸ߣ��Ǿʹ�ֱ����
		if (pending_top == nullptr && pending_bottom == nullptr
			&& top == nullptr && scale_top == nullptr && bottom == nullptr && scale_bottom == nullptr 
			&& (height!=nullptr||scale_height!=nullptr)) {
			region.y = parent_region.y + (parent_region.h - region.h) / 2;
		}

		//������left��scale_left��pending_leftʱ������x
		if (pending_left != nullptr) {
			region.x = pending_left->region.x + pending_left->region.w;
		}
		else if (left != nullptr) {
			region.x = parent_region.x + *left;
		}
		else if (scale_left != nullptr) {
			region.x = parent_region.x + parent_region.w * (*scale_left);
		}

		//������top��scale_top��pending_topʱ������y
		if (pending_top != nullptr) {
			region.y = pending_top->region.y + pending_top->region.h;
		}
		if (top != nullptr) {
			region.y = parent_region.y + *top;
		}
		else if (scale_top != nullptr) {
			region.y = parent_region.y + parent_region.h * (*scale_top);
		}

		//������right��scale_rightʱ�����ͬʱ������width��scale_width����ʱleft��Ч��������x��������left����w
		//����pending_right pending_bottom��Ҫregion.h region.w,�ʷ��ں������
		if (pending_right == nullptr) {
			if (right != nullptr) {
				if (width!=nullptr||scale_width!=nullptr) {
					region.x = parent_region.x + parent_region.w-region.w-*right;
				}
				else {
					region.w = parent_region.x + parent_region.w - region.x - *right;
				}
			}
			else if (scale_right != nullptr) {
				if (width != nullptr||scale_width!=nullptr) {
					region.x = parent_region.x + parent_region.w - region.w - parent_region.w * (*scale_right);
				}
				else {
					region.w = parent_region.x + parent_region.w - region.x - parent_region.w * (*scale_right);
				}
			}
		}
		//������bottom��scale_bottomʱ�����ͬʱ������height��scale_height����ʱtop��Ч��������x��������top����h
		if (pending_bottom == nullptr) {
			if (bottom != nullptr) {
				if (height != nullptr || scale_height != nullptr) {
					region.y = parent_region.y + parent_region.h - region.h - *bottom;
				}
				else {
					region.h = parent_region.y + parent_region.h - region.y - *bottom;
				}
			}
			else if (scale_bottom != nullptr) {
				if (height != nullptr || scale_height != nullptr) {
					region.y = parent_region.y + parent_region.h - region.h - parent_region.h * (*scale_bottom);
				}
				else {
					region.h = parent_region.y + parent_region.h - region.y - parent_region.h * (*scale_bottom);
				}
			}
		}

		//���Ƴߴ���min��max֮�䣬û������min max������
		if (max_width != nullptr) {
			if (region.w > * max_width) region.w = *max_width;
		}
		else if (scale_max_width != nullptr) {
			if (region.w / parent_region.w > * scale_max_width)region.w = parent_region.w * (*scale_max_width);
		}

		if (min_width != nullptr) {
			if (region.w < *min_width) region.w = *min_width;
		}
		else if (scale_min_width != nullptr) {
			if (region.w / parent_region.w < *scale_min_width)region.w = parent_region.w * (*scale_min_width);
		}

		if (max_height != nullptr) {
			if (region.h > * max_height) region.h = *max_height;
		}
		else if (scale_max_height != nullptr) {
			if (region.h / parent_region.h > * scale_max_height)region.h = parent_region.h * (*scale_max_height);
		}

		if (min_height != nullptr) {
			if (region.h < *min_height) region.h = *min_height;
		}
		else if (scale_min_height != nullptr) {
			if (region.h / parent_region.h < *scale_min_height)region.h = parent_region.h * (*scale_min_height);
		}

		//����right��bottom������layout ��Ϊ������Ҫ��֪w h���Է��ڳߴ����Ƽ���֮��
		if (pending_right != nullptr) {
			region.x = pending_right->region.x - region.w;
		}
		if (pending_bottom != nullptr) {
			region.y = pending_bottom->region.y - region.h;
		}

		//ѡ����ʵ���layout
		Size size;
		bool flag = false;
		size.height = region.h;
		size.width = region.w;
		size.scale_height = region.h / parent_region.h;
		size.scale_width = region.w / parent_region.w;
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
		//���region�������
		//....
		//֪ͨˢ��
	}
	void setChild(Layout* layout) {
		if(child!=nullptr)delete child;
		this->child=layout;
		child->parent=this;
		display=child;
		//���region�������
		//resetRegion
		//���region�������
		//....
		//֪ͨˢ��
	}
	void setMinChild(Layout* layout, Condition condition) {
		if(min_child!=nullptr)delete min_child;
		this->min_child=layout;
		if(layout!=nullptr)layout->parent=this;
		this->min_condition=condition;
		//���region�������
		//resetRegion
		//���region�������
		//....
		//֪ͨˢ��
	}
	void setMaxChild(Layout* layout, Condition condition) {
		if(max_child!=nullptr)delete max_child;
		this->max_child=layout;
		if (layout != nullptr)layout->parent=this;
		this->max_condition=condition;
		//���region�������
		//resetRegion
		//���region�������
		//....
		//֪ͨˢ��
	}
	//���ú�Ŀ��layout����������Edge::left�ǽ���ʵ������ߺ�Ŀ��layout���ұ߶���
	//ͬʱĿ��layout��Ϊ��ʵ����pending_left
	//��ʵ����ΪĿ��layout��neighbor_right
	void setNeighbor(Layout* layout, Edge edge) {
		switch (edge) {
		case Edge::left:
			if(pending_left!=nullptr)delete pending_left;
			pending_left=layout;
			layout->neighbor_right=this;
			break;
		case Edge::top:
			if(pending_top!=nullptr)delete pending_top;
			pending_top=layout;
			layout->neighbor_bottom=this;
			break;
		case Edge::right:
			if(pending_right!=nullptr)delete pending_right;
			pending_right=layout;
			layout->neighbor_left=this;
			break;
		case Edge::bottom:
			if(pending_bottom!=nullptr)delete pending_bottom;
			pending_bottom=layout;
			layout->neighbor_top=this;
			break;
		}
		//���region�������
		//resetRegion
		//���region�������
		//....
		//֪ͨˢ��
	}
	
};

#endif
