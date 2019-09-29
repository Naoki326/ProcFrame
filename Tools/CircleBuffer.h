#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <memory>

namespace MyAlg
{
	using std::shared_ptr;

	//循环Buffer，Discard的数据不删除，留作存储后续数据
	template<class T>
	class CircleBuffer
	{
	public:
		//数据的状态
		enum bf_state : unsigned char
		{
			RESIZED,		//表示变更的数据
			DONE			//表示通知完变更，正常使用的数据
		};

	public:
		CircleBuffer(int size);

		//获取该结构的指针
		T* GetEd();
		T* GetSt();

		//判断数据结构是否为空
		bool IsEmpty();

		//增加数据
		//数据传入基本属于顺序
		//采用从后往前插入排序的方法
		bool PushBack(const T& t);
		bool PushBack(T&& t);

		//删除数据
		//通知，改变数据的首位
		bool PopFront();

		//达到数据的末尾
		bool EndOfBuffer(const T * p) const;
		bool StartOfBuffer(const T * p) const;

		//下面两个函数是仿链表函数，这里只用了LastPtr进行遍历
		//找到上一个已被使用的数据位置
		T * LastPtr(const T* p) const;
		//找到下一个已被使用的数据位置
		T * NextPtr(const T* p) const;

		unsigned int GetLen() const;

		unsigned int GetSize() const;

		//扩大循环buffer的size为输入size
		bool ResetSize(const unsigned int size);

		//若条件允许，将缓存设置为输入的大小
		//返回0，设置成功
		//返回1，缓存越过0点，设置未成功
		//返回2，缓存大小大于输入大小，设置未成功
		int CollectGC(const unsigned int size);

		T& At(const int i) const;
		T& operator[](const int i) const;

	private:
		//堆上数据的长度
		unsigned int pts_len_;

		//读入数据的总长度
		unsigned int size_;
		//数据指针
		shared_ptr<T> pts_;
		//头位置
		T* pts_st_;
		//尾位置
		T* pts_ed_;

		//状态
		bf_state bf_state_;

		int obsolete_l_part_size;
		int obsolete_r_part_size;
	};





#pragma region CircleBuffer

	template<class T>
	CircleBuffer<T>::CircleBuffer(int size)
	{
		pts_len_ = 0;
		size_ = size;
		pts_ = std::shared_ptr<T>(new T[size_], [](T* p) { delete[] p; });
		pts_st_ = pts_.get();
		pts_ed_ = pts_.get();
		bf_state_ = DONE;
	}

	//判断数据结构是否为空
	template<class T>
	bool CircleBuffer<T>::IsEmpty()
	{
		return (pts_len_ == 0);
	}

	template<class T>
	T* CircleBuffer<T>::GetEd()
	{
		return pts_ed_;
	}

	template<class T>
	T* CircleBuffer<T>::GetSt()
	{
		return pts_st_;
	}

	//增加数据
	//数据传入基本属于顺序
	//采用从后往前插入排序的方法
	template<class T>
	bool CircleBuffer<T>::PushBack(T&& t)
	{

		//判断是否为空
		if (IsEmpty() == true)
		{
			//构造赋值
			(*pts_ed_) = std::move(t);
			pts_len_++;
			return true;
		}
		//判断循环buffer是否已满
		else if (
			//头尾指针相撞
			(EndOfBuffer(pts_ed_) == false && (pts_ed_ + 1)->GetState() == T::USED)
			//达到尾部
			|| (EndOfBuffer(pts_ed_) == true && pts_->GetState() == T::USED)
			)
		{
			if (ResetSize(size_ * 2) == false)
				return false;
		}

		//是否位于尾部
		if (EndOfBuffer(pts_ed_) == true)
		{
			pts_ed_ = pts_.get();
		}
		else
		{
			pts_ed_ += 1;
		}
		(*pts_ed_) = std::move(t);
		pts_len_++;

		return true;
	}

	template<class T>
	bool CircleBuffer<T>::PushBack(const T& t)
	{
		//判断是否为空
		if (IsEmpty() == true)
		{
			//构造赋值
			(*pts_ed_) = t;
			pts_len_++;
			return true;
		}
		//判断循环buffer是否已满
		else if (
			//头尾指针相撞
			(EndOfBuffer(pts_ed_) == false && (pts_ed_ + 1)->GetState() == T::USED)
			//达到尾部
			|| (EndOfBuffer(pts_ed_) == true && pts_->GetState() == T::USED)
			)
		{
			if (ResetSize(size_ * 2) == false)
				return false;
		}

		//是否位于尾部
		if (EndOfBuffer(pts_ed_) == true)
		{
			pts_ed_ = pts_.get();
		}
		else
		{
			pts_ed_ += 1;
		}

		(*pts_ed_) = t;
		pts_len_++;

		return true;
	}



	template<class T>
	inline bool CircleBuffer<T>::PopFront()
	{
		T& mid = this->At(0);
		if (pts_len_ == 1)
		{
			pts_st_ = pts_.get();
			pts_ed_ = pts_.get();
		}
		else
		{
			pts_st_ = NextPtr(pts_st_);
		}
		mid.SetState(T::DISCARD);
		pts_len_--;
		return true;
	}

	//达到数据的末尾
	template<class T>
	bool CircleBuffer<T>::EndOfBuffer(const T * p) const
	{
		return size_ - 1 == (p - pts_.get());
	}

	template<class T>
	bool CircleBuffer<T>::StartOfBuffer(const T * p) const
	{
		return 0 == (p - pts_.get());
	}

	//找到上一个已被使用的数据位置
	template<class T>
	T* CircleBuffer<T>::LastPtr(const T* p) const
	{
		T * last_p = const_cast<T*>(p);
		do
		{
			if (StartOfBuffer(last_p) == true)
			{
				last_p = pts_.get() + size_ - 1;
			}
			else
			{
				last_p = const_cast<T*>(last_p - 1);
			}
		} while (last_p->GetState() != T::USED);

		return last_p;
	}

	//找到下一个已被使用的数据位置
	template<class T>
	T* CircleBuffer<T>::NextPtr(const T* p) const
	{
		T * next_p = const_cast<T*>(p);
		do
		{
			if (EndOfBuffer(next_p) == true)
			{
				next_p = pts_.get();
			}
			else
			{
				next_p = const_cast<T*>(next_p + 1);
			}
		} while (next_p->GetState() != T::USED);

		return next_p;
	}

	template<class T>
	unsigned int CircleBuffer<T>::GetLen() const
	{
		return pts_len_;
	}

	template<class T>
	unsigned int CircleBuffer<T>::GetSize() const
	{
		return size_;
	}

	//扩大循环buffer的size为输入size
	template<class T>
	bool CircleBuffer<T>::ResetSize(const unsigned int size)
	{
		if (size < size_)
			return false;

		shared_ptr<T> pts_obsolete = pts_;
		pts_.reset(new T[size], [](T* p) { delete[] p; });

		if (pts_ed_ - pts_st_ > 0)
		{
			//buffer没有跨越0点
			obsolete_l_part_size = 0;
			obsolete_r_part_size = size_;
			std::memcpy(pts_.get(), pts_st_, pts_len_ * sizeof(T));
		}
		else
		{
			//buffer跨越了0点
			obsolete_l_part_size = pts_ed_ - pts_.get() + 1;
			obsolete_r_part_size = size_ - obsolete_l_part_size;
			if (obsolete_l_part_size == 0 || obsolete_r_part_size == 0)
			{
				std::memcpy(pts_.get(), pts_obsolete.get(), size_ * sizeof(T));
			}
			else
			{
				std::memcpy(pts_.get(), pts_obsolete.get() + obsolete_l_part_size, obsolete_r_part_size * sizeof(T));
				std::memcpy(pts_.get() + obsolete_r_part_size, pts_obsolete.get(), obsolete_l_part_size * sizeof(T));
			}
		}
		pts_st_ = pts_.get();
		pts_ed_ = pts_.get() + pts_len_-1;
		size_ = size;
		bf_state_ = RESIZED;

		return true;
	}

	//若条件允许，将缓存设置为输入的大小
	//返回0，设置成功
	//返回1，缓存越过0点，设置未成功
	//返回2，缓存大小大于输入大小或输入大小等于0，设置未成功
	template<class T>
	int CircleBuffer<T>::CollectGC(const unsigned int size)
	{
		if (size == 0)
			return 2;
		int pts_len = pts_ed_ - pts_st_+1;
		if (pts_len < size)
		{
			shared_ptr<T> pts_obsolete = pts_;
			obsolete_l_part_size = 0;
			obsolete_r_part_size = size;
			pts_.reset(new T[size], [](T* p) { delete[] p; });
			std::memcpy(pts_.get(), pts_obsolete.get() + obsolete_l_part_size, pts_len * sizeof(T));
			pts_st_ = pts_.get();
			pts_ed_ = pts_.get() + pts_len-1;
			size_ = size;
			bf_state_ = RESIZED;
			return 0;
		}
		else if (pts_len < 0)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	template<class T>
	T & CircleBuffer<T>::At(const int i) const
	{
		if (i >= pts_len_ || i < 0)
		{
			return *pts_st_;
		}
		int offset = (pts_st_ - pts_.get()) + i - (size_ - 1);
		if (offset > 0)
			return *(pts_.get() + offset);
		else
			return *(pts_st_ + i);
	}

	template<class T>
	T & CircleBuffer<T>::operator[](const int i) const
	{
		return At(i);
	}

#pragma endregion CircleBuffer

}

#endif







/***
*                    .::::.
*                  .::::::::.
*                 ::::::::::: FUCK YOU
*             ..:::::::::::'
*           '::::::::::::'
*             .::::::::::
*        '::::::::::::::..
*             ..::::::::::::.
*           ``::::::::::::::::
*            ::::``:::::::::'        .:::.
*           ::::'   ':::::'       .::::::::.
*         .::::'      ::::     .:::::::'::::.
*        .:::'       :::::  .:::::::::' ':::::.
*       .::'        :::::.:::::::::'      ':::::.
*      .::'         ::::::::::::::'         ``::::.
*  ...:::           ::::::::::::'              ``::.
* ```` ':.          ':::::::::'                  ::::..
*                    '.:::::'                    ':'````..
*/
