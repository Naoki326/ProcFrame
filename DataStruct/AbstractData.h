#ifndef ABSTRACTDATA_H
#define ABSTRACTDATA_H

#include <iostream>
#include <numeric>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>

#include "../Tools/CircleBuffer.h"
#include "../Tools/Threads.h"
#include "../Frame/Factory.h"


namespace MyAlg
{
	using std::string;
	using std::shared_ptr;
	
	//最底层数据结构，(x,y)
	//考虑到32位系统，结构体对齐，单个数据长度4个字节+基类加锁4个字节共8字节
	//预计会在栈上动态生成该数据结构
	//cmd2用64位数据保存位置，这里使用16位的unsigned short保存x_
	//会在上一级结构中保存一个基位置base_pos_，与cmd2的位置关联
	//x_是一个位置偏移量，数据的真实位置是base_pos_+x_
	//采用对象锁的话，只有在访问到该对象时线程才会锁住
	//不用考虑多线程时，用class lowest_level_pt : public SingleThreaded<lowest_level_pt>
	class lowest_level_pt : public ClassLevelLockable<lowest_level_pt>
	{
	public:
		//数据的状态
		enum pt_state : unsigned char
		{
			UNINITIALIZED,	//表示未赋值过
			USED,			//表示正在使用的数据
			DISCARD			//表示不需要的数据
		};

	public:
		//初始化
		inline lowest_level_pt() : x_(0), y_(0), pt_state_(UNINITIALIZED) {}
		inline lowest_level_pt(const unsigned int x, const unsigned char y, const pt_state state)
		{
			x_ = x;
			y_ = y;
			pt_state_ = state;
		}
		inline lowest_level_pt(const lowest_level_pt & t) : x_(t.x_), y_(t.y_), pt_state_(t.pt_state_) {}
		inline lowest_level_pt& operator=(const lowest_level_pt & data)
		{
			Lock lock();
			x_ = data.x_;
			y_ = data.y_;
			pt_state_ = data.pt_state_;
			return *this;
		}
		//类中没有用到指针，可以不用定义移动构造、移动赋值
		//下面移动构造、移动赋值仅作为备用
		inline lowest_level_pt(lowest_level_pt && t) : x_(t.x_), y_(t.y_), pt_state_(t.pt_state_) 
		{}
		inline lowest_level_pt& operator=(lowest_level_pt && data)
		{
			x_ = data.x_;
			y_ = data.y_;
			pt_state_ = data.pt_state_;
			return *this;
		}
		inline void Set(const unsigned int x, const unsigned char y, const pt_state state)
		{
			Lock lock();
			x_ = x;
			y_ = y;
			pt_state_ = state;
		}
		inline unsigned int GetX() const
		{
			Lock lock();
			return x_;
		}
		inline unsigned char GetY() const
		{
			Lock lock();
			return y_;
		}
		inline pt_state GetState() const
		{
			Lock lock();
			return pt_state_;
		}
		inline void SetState(lowest_level_pt::pt_state state)
		{
			Lock lock();
			pt_state_ = state;
		}

	private:
		unsigned int x_;	//2字节
		unsigned char y_;	//1字节
		pt_state pt_state_;	//1字节
		//基类有4字节
	};

	//用循环buffer封装底层数据
	typedef CircleBuffer<lowest_level_pt> circle_buffer;

	// 算法类处理的数据结构
	class AbstractData
	{
	protected:
		shared_ptr<circle_buffer> cb_ptr_;
		unsigned char by_channel_id_;
		//基位置
		shared_ptr<unsigned __int64> base_pos_;
		//避免RTTI
		//string type_;

	private:
		AbstractData(){}

	public:
		AbstractData(const AbstractData & data)
			: cb_ptr_(data.cb_ptr_), by_channel_id_(data.by_channel_id_), 
			base_pos_(data.base_pos_)
		{}
		AbstractData(AbstractData && data)
			: cb_ptr_(data.cb_ptr_), by_channel_id_(data.by_channel_id_),
			base_pos_(data.base_pos_)
		{
			data.cb_ptr_.reset();
		}
		AbstractData(unsigned char id)
			: by_channel_id_(id)
		{
			base_pos_.reset(new unsigned __int64(0));
		}

		void SetBasePos(unsigned __int64 base_pos)
		{
			base_pos_.reset(new unsigned __int64(base_pos));
		}

		unsigned char GetChannel()
		{
			return by_channel_id_;
		}

	};

	//单通道的所有数据集合的托管类
	//插入及查找方式使循环buff内的数据均为顺序数据
	class channel_level_data : public AbstractData
	{
	public:
		//初始化
		inline channel_level_data(unsigned char id, unsigned int size)
			: AbstractData(id)
		{
			//这里是数据的产生源
			cb_ptr_.reset(new circle_buffer(size));
		}

		//提供下标访问接口
		//lowest_level_pt& At(const int i) const;
		//lowest_level_pt& operator[](const int i) const;

		inline unsigned __int64 GetX(const int i) const
		{
			return cb_ptr_->At(i).GetX() + (*base_pos_);
		}
		inline unsigned char GetY(const int i) const
		{
			return cb_ptr_->At(i).GetY();
		}
		inline lowest_level_pt::pt_state GetState(const int i) const
		{
			return cb_ptr_->At(i).GetState();
		}
		inline void DiscardFront()
		{
			cb_ptr_->PopFront();
			return;
		}

		//取得数据长度
		unsigned int GetLen();

		inline shared_ptr<circle_buffer> GetPtr() const
		{ return cb_ptr_; }
		
		//设置偏移量
		inline void SetBasePos(unsigned __int64 base_pos)
		{
			for (int i = 0; i < cb_ptr_->GetLen(); i++)
			{
				cb_ptr_->At(i).Set(GetX(i) - base_pos, GetY(i), GetState(i));
			}
			base_pos_.reset(new unsigned __int64(base_pos));
		}

		//向表中插入数据，并使数据按x_顺序排列
		//插入同时排序
		//真实位置是base_pos_+x
		bool Insert(unsigned __int64 x, unsigned char y);

		//顺序表二分查找
		int FindX(unsigned __int64 x);
	};


	//单通道特征
	class sc_feature
	{
	public:
		//Features
		double avr_depth_;
		short max_depth_;
		short height_;
		int width_;
		int dot_count_;
		double slope_;

		unsigned long long left_x_;
		unsigned long long  right_x_;
		short up_;
		short bottom_;

		bool IsEqual(sc_feature & rhs)
		{
			return this->avr_depth_ == rhs.avr_depth_
				&& this->max_depth_ == rhs.max_depth_
				&& this->height_ == rhs.height_
				&& this->width_ == rhs.width_
				&& this->dot_count_ == rhs.dot_count_
				&& this->slope_ == rhs.slope_
				&& this->left_x_ == rhs.left_x_
				&& this->right_x_ == rhs.right_x_
				&& this->up_ == rhs.up_
				&& this->bottom_ == rhs.bottom_;
		}
	};


	//单通道的数据集合的托管类2
	//预计算法处理类将使用该数据
	class deformation_data : public AbstractData
	{
	private:
		//指示数据所在位置的索引
		std::vector<int> pts_vec_;

		//禁赋值函数

	public:
		sc_feature scfeature_;
		deformation_data & operator=(const deformation_data & rhs)
		{
			AbstractData::AbstractData(rhs);
			this->pts_vec_.assign(rhs.pts_vec_.begin(), rhs.pts_vec_.end());
			return *this;
		}
		
		//初始化
		inline deformation_data(const AbstractData& rhs)
			: AbstractData(rhs)
		{
		}

		//拷贝构造，索引深拷贝
		inline deformation_data(const deformation_data& rhs)
			: AbstractData(rhs.by_channel_id_)
		{
			this->cb_ptr_ = rhs.cb_ptr_;
			this->base_pos_ = rhs.base_pos_;
			this->pts_vec_ = std::vector<int>(rhs.pts_vec_);
			this->scfeature_ = rhs.scfeature_;
		}

		//拷贝构造，右值方式
		inline deformation_data(deformation_data&& rhs)
			: AbstractData(rhs.by_channel_id_)
		{
			this->cb_ptr_ = rhs.cb_ptr_;
			this->base_pos_ = rhs.base_pos_;
			rhs.pts_vec_.swap(this->pts_vec_);
			this->scfeature_ = rhs.scfeature_;
		}

		//拷贝构造，索引深拷贝
		inline deformation_data(const deformation_data * rhs)
			: AbstractData(rhs->by_channel_id_)
		{
			this->cb_ptr_ = rhs->cb_ptr_;
			this->base_pos_ = rhs->base_pos_;
			this->pts_vec_ = std::vector<int>(rhs->pts_vec_);
			this->scfeature_ = rhs->scfeature_;
		}

		inline unsigned int GetSize() const
		{ return pts_vec_.size(); }
		//提供下标访问接口
	private:
		lowest_level_pt& PriAt(const int i) const;
	public:
		inline unsigned __int64 GetEndX() const
		{
			return GetX(this->GetSize() - 1);
		}
		inline unsigned char GetEndY() const
		{
			return GetY(this->GetSize() - 1);
		}
		inline lowest_level_pt::pt_state GetEndState() const
		{
			return GetState(this->GetSize() - 1);
		}

		inline unsigned __int64 GetX(const int i) const
		{
			return PriAt(i).GetX() + (*base_pos_);
		}
		inline unsigned char GetY(const int i) const
		{
			return PriAt(i).GetY();
		}
		inline lowest_level_pt::pt_state GetState(const int i) const
		{
			return PriAt(i).GetState();
		}
		inline void SetState(const int i,
			lowest_level_pt::pt_state state)
		{
			return PriAt(i).SetState(state);
		}

		
		inline void SetIndVec(const std::vector<int>& pts_vec)
		{ this->pts_vec_ = std::vector<int>(pts_vec); }

		inline const std::vector<int> GetIndVec() const
		{ return this->pts_vec_; }

		inline void InsertData(int ind)
		{ pts_vec_.push_back(ind); }

		inline void Clear()
		{ pts_vec_.clear(); }

		inline bool Swap(int i, int j)
		{
			if (i >= 0 && j >= 0 && i < GetSize() && j < GetSize())
			{
				auto tmp = std::move(pts_vec_[i]);
				pts_vec_[i] = std::move(pts_vec_[j]);
				pts_vec_[j] = std::move(tmp);
				return true;
			}else return false;
		}

		template<class _Pr>
		void Sort(int begin, int len, _Pr _Pred)
		{
			std::vector<int> mid, mid2;
			for (int i = 0; i < pts_vec_.size(); i++)
				mid.push_back(i);
			std::sort(mid.begin() + begin, mid.end(), _Pred);
			mid2.resize(pts_vec_.size());
			for (int i = 0; i < pts_vec_.size(); i++)
			{
				mid2[i] = pts_vec_[mid[i]];
			}
			pts_vec_.assign(mid2.begin(), mid2.end());
		}

		template<class _Pr>
		void Sort(_Pr _Pred)
		{ std::sort(pts_vec_.begin(), pts_vec_.end(), _Pred); }

	};

	//单通道的数据集合的托管类3
	//预计算法处理类将使用该数据
	class multi_box
	{
	protected:

	public:
		std::vector<deformation_data> deform_vec_;
		multi_box()
		{}

		multi_box(const multi_box & data)
		{
			deform_vec_ = std::vector<deformation_data>(data.deform_vec_);
		}

		multi_box(const deformation_data & data)
		{
			deform_vec_.clear();
			deform_vec_.push_back(data);
		}

		void Insert(const deformation_data & data)
		{
			deform_vec_.push_back(data);
		}

		unsigned int GetLen() const 
		{
			return deform_vec_.size();
		}

		unsigned int DotsLen() const
		{
			int len = 0;
			for (int i = 0; i < deform_vec_.size(); i++)
			{
				len += deform_vec_.size();
			}
			return len;
		}
	};


	//单通道特征
	class mc_feature
	{
	public:
		double avr_depth_;
		int dot_count_;

		unsigned long long left_;
		unsigned long long  right_;
		short up_;
		short bottom_;
	};


	class defect_box : public multi_box
	{
	private:

	public:
		mc_feature mcfeature_;
		unsigned char condition_id_;

		defect_box()
			:multi_box()
		{
			condition_id_ = 0;
			mcfeature_.up_ = 1000;
			mcfeature_.bottom_ = 0;
			mcfeature_.left_ = std::numeric_limits<unsigned long long>::max();
			mcfeature_.right_ = 0;
			mcfeature_.dot_count_ = 0;
			mcfeature_.avr_depth_ = 0;
		}

		defect_box(const defect_box & data)
			:multi_box(data)
		{
			this->condition_id_ = data.condition_id_;
			mcfeature_.up_ = data.mcfeature_.up_;
			mcfeature_.bottom_ = data.mcfeature_.bottom_;
			mcfeature_.left_ = data.mcfeature_.left_;
			mcfeature_.right_ = data.mcfeature_.right_;
			mcfeature_.dot_count_ = data.mcfeature_.dot_count_;
			mcfeature_.avr_depth_ = data.mcfeature_.avr_depth_;
		}

		defect_box(const deformation_data & data)
			:multi_box(data)
		{
			mcfeature_.up_ = 1000;
			mcfeature_.bottom_ = 0;
			mcfeature_.left_ = std::numeric_limits<unsigned long long>::max();
			mcfeature_.right_ = 0;
			mcfeature_.dot_count_ = 0;
			mcfeature_.avr_depth_ = 0;
		}

		defect_box operator-(defect_box& rhs)
		{
			defect_box returnval(*this);
			for (auto deform_r : rhs.deform_vec_)
			{
				auto iter = returnval.deform_vec_.begin();
				while (true)
				{
					find_if(iter, returnval.deform_vec_.end(), [&deform_r](deformation_data deform_l)
					{
						if (deform_l.scfeature_.IsEqual(deform_r.scfeature_)
							&& deform_l.GetChannel() == deform_r.GetChannel())
							return true;
					});
					if (iter != returnval.deform_vec_.end())
						iter = returnval.deform_vec_.erase(iter);
					else
						break;
				}
			}
			return returnval;
		}

		void UpdateFeature()
		{
			mcfeature_.dot_count_ = 0;
			mcfeature_.avr_depth_ = 0;
			for (int i = 0; i < deform_vec_.size(); i++)
			{
				for (int j = 0; j < deform_vec_[i].GetSize(); j++)
				{
					if (deform_vec_[i].GetY(j) < mcfeature_.up_)
						mcfeature_.up_ = deform_vec_[i].GetY(j);
					if (deform_vec_[i].GetY(j) > mcfeature_.bottom_)
						mcfeature_.bottom_ = deform_vec_[i].GetY(j);
					if (deform_vec_[i].GetX(j) < mcfeature_.left_)
						mcfeature_.left_ = deform_vec_[i].GetX(j);
					if (deform_vec_[i].GetX(j) > mcfeature_.right_)
						mcfeature_.right_ = deform_vec_[i].GetX(j);
					mcfeature_.dot_count_++;
					mcfeature_.avr_depth_ += deform_vec_[i].GetY(j);
				}
			}
			mcfeature_.avr_depth_ /= mcfeature_.dot_count_;
		}

		bool IsChannelIn(int id)
		{
			auto iter = find_if(this->deform_vec_.begin(), this->deform_vec_.end(), [id](deformation_data d)
			{
				return d.GetChannel() == id;
			});
			if (iter != this->deform_vec_.end())
				return true;
			else
				return false;
		}
		bool IsChannelIn(std::vector<int> ids)
		{
			auto iter = find_if(this->deform_vec_.begin(), this->deform_vec_.end(), [ids](deformation_data d)
			{
				for (auto id : ids)
				{
					if (d.GetChannel() == id)
						return true;
				}
				return false;
			});
			if (iter != this->deform_vec_.end())
				return true;
			else
				return false;
		}


		unsigned long long GetStart()
		{
			return mcfeature_.left_;
		}
		unsigned long long GetCenter()
		{
			return (mcfeature_.left_ + mcfeature_.right_) / 2;
		}

		unsigned long long GetEnd()
		{
			return mcfeature_.right_;
		}

		unsigned short GetDepthStart()
		{
			return mcfeature_.up_;
		}
		unsigned short GetDepthCenter()
		{
			return (mcfeature_.up_+ mcfeature_.bottom_)/2;
		}

		unsigned short GetDepthEnd()
		{
			return mcfeature_.bottom_;
		}

		int HorLen()
		{
			return (static_cast<int>(mcfeature_.right_) - mcfeature_.left_);
		}

		char DepthLen() 
		{
			return (static_cast<char>(mcfeature_.bottom_) - mcfeature_.up_);
		}

	};




}
#endif