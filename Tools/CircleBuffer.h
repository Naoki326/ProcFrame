#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <memory>

namespace MyAlg
{
	using std::shared_ptr;

	//ѭ��Buffer��Discard�����ݲ�ɾ���������洢��������
	template<class T>
	class CircleBuffer
	{
	public:
		//���ݵ�״̬
		enum bf_state : unsigned char
		{
			RESIZED,		//��ʾ���������
			DONE			//��ʾ֪ͨ����������ʹ�õ�����
		};

	public:
		CircleBuffer(int size);

		//��ȡ�ýṹ��ָ��
		T* GetEd();
		T* GetSt();

		//�ж����ݽṹ�Ƿ�Ϊ��
		bool IsEmpty();

		//��������
		//���ݴ����������˳��
		//���ôӺ���ǰ��������ķ���
		bool PushBack(const T& t);
		bool PushBack(T&& t);

		//ɾ������
		//֪ͨ���ı����ݵ���λ
		bool PopFront();

		//�ﵽ���ݵ�ĩβ
		bool EndOfBuffer(const T * p) const;
		bool StartOfBuffer(const T * p) const;

		//�������������Ƿ�������������ֻ����LastPtr���б���
		//�ҵ���һ���ѱ�ʹ�õ�����λ��
		T * LastPtr(const T* p) const;
		//�ҵ���һ���ѱ�ʹ�õ�����λ��
		T * NextPtr(const T* p) const;

		unsigned int GetLen() const;

		unsigned int GetSize() const;

		//����ѭ��buffer��sizeΪ����size
		bool ResetSize(const unsigned int size);

		//��������������������Ϊ����Ĵ�С
		//����0�����óɹ�
		//����1������Խ��0�㣬����δ�ɹ�
		//����2�������С���������С������δ�ɹ�
		int CollectGC(const unsigned int size);

		T& At(const int i) const;
		T& operator[](const int i) const;

	private:
		//�������ݵĳ���
		unsigned int pts_len_;

		//�������ݵ��ܳ���
		unsigned int size_;
		//����ָ��
		shared_ptr<T> pts_;
		//ͷλ��
		T* pts_st_;
		//βλ��
		T* pts_ed_;

		//״̬
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

	//�ж����ݽṹ�Ƿ�Ϊ��
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

	//��������
	//���ݴ����������˳��
	//���ôӺ���ǰ��������ķ���
	template<class T>
	bool CircleBuffer<T>::PushBack(T&& t)
	{

		//�ж��Ƿ�Ϊ��
		if (IsEmpty() == true)
		{
			//���츳ֵ
			(*pts_ed_) = std::move(t);
			pts_len_++;
			return true;
		}
		//�ж�ѭ��buffer�Ƿ�����
		else if (
			//ͷβָ����ײ
			(EndOfBuffer(pts_ed_) == false && (pts_ed_ + 1)->GetState() == T::USED)
			//�ﵽβ��
			|| (EndOfBuffer(pts_ed_) == true && pts_->GetState() == T::USED)
			)
		{
			if (ResetSize(size_ * 2) == false)
				return false;
		}

		//�Ƿ�λ��β��
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
		//�ж��Ƿ�Ϊ��
		if (IsEmpty() == true)
		{
			//���츳ֵ
			(*pts_ed_) = t;
			pts_len_++;
			return true;
		}
		//�ж�ѭ��buffer�Ƿ�����
		else if (
			//ͷβָ����ײ
			(EndOfBuffer(pts_ed_) == false && (pts_ed_ + 1)->GetState() == T::USED)
			//�ﵽβ��
			|| (EndOfBuffer(pts_ed_) == true && pts_->GetState() == T::USED)
			)
		{
			if (ResetSize(size_ * 2) == false)
				return false;
		}

		//�Ƿ�λ��β��
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

	//�ﵽ���ݵ�ĩβ
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

	//�ҵ���һ���ѱ�ʹ�õ�����λ��
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

	//�ҵ���һ���ѱ�ʹ�õ�����λ��
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

	//����ѭ��buffer��sizeΪ����size
	template<class T>
	bool CircleBuffer<T>::ResetSize(const unsigned int size)
	{
		if (size < size_)
			return false;

		shared_ptr<T> pts_obsolete = pts_;
		pts_.reset(new T[size], [](T* p) { delete[] p; });

		if (pts_ed_ - pts_st_ > 0)
		{
			//bufferû�п�Խ0��
			obsolete_l_part_size = 0;
			obsolete_r_part_size = size_;
			std::memcpy(pts_.get(), pts_st_, pts_len_ * sizeof(T));
		}
		else
		{
			//buffer��Խ��0��
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

	//��������������������Ϊ����Ĵ�С
	//����0�����óɹ�
	//����1������Խ��0�㣬����δ�ɹ�
	//����2�������С���������С�������С����0������δ�ɹ�
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
