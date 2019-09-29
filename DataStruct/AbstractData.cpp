#include "AbstractData.h"


namespace MyAlg
{

#pragma region channel_level_data
	
	bool channel_level_data::Insert(unsigned __int64 x, unsigned char y)
	{
		unsigned int byx = static_cast<unsigned int>(x - *base_pos_);
		//表中无数据
		if (cb_ptr_->IsEmpty() == true)
			return cb_ptr_->PushBack(lowest_level_pt(byx, y, lowest_level_pt::USED));

		//表中存在数据，先在结尾增加一个数据
		lowest_level_pt t = *cb_ptr_->GetEd();
		if (cb_ptr_->PushBack(std::move(t)) == false)
			return false;


		//插入到合适位置
		for (auto iter = cb_ptr_->GetEd(); ; iter = cb_ptr_->LastPtr(iter))
		{
			if (iter->GetX() <= byx)
			{
				iter->Set(byx, y, lowest_level_pt::USED);
				break;
			}
			else if (iter - cb_ptr_->GetSt() == 0)
			{
				iter->Set(byx, y, lowest_level_pt::USED);
				break;
			}
			else
			{
				iter->Set((iter - 1)->GetX(), (iter - 1)->GetY(), lowest_level_pt::USED);
			}
		}

		return true;
	}


// 	lowest_level_pt & channel_level_data::At(const int i) const
// 	{
// 		// TODO: 在此处插入 return 语句
// 		return cb_ptr_->At(i);
// 	}
// 
// 	lowest_level_pt & channel_level_data::operator[](const int i) const
// 	{
// 		return this->At(i);
// 	}

	int channel_level_data::FindX(unsigned __int64 x)
	{
		unsigned short byx = static_cast<unsigned short>(x - *base_pos_);
		int low, high, mid;
		low = 0;
		high = cb_ptr_->GetLen();

		while (low <= high)
		{
			mid = (low + high) / 2;
			if (this->GetX(mid) == x)
			{
				return mid;
			}
			if (this->GetX(mid) > x)
			{
				high = mid - 1;
			}
			if (this->GetX(mid) < x)
			{
				low = mid + 1;
			}
		}
		return -1;
	}

	unsigned int channel_level_data::GetLen()
	{
		return cb_ptr_->GetLen();
	}
	
#pragma endregion channel_level_data



#pragma region deformation_data

	lowest_level_pt & deformation_data::PriAt(const int i) const
	{
		if (i < 0 || i >= pts_vec_.size())
		{
			return cb_ptr_->At(this->pts_vec_[0]);
		}
		else
		{
			return cb_ptr_->At(this->pts_vec_[i]);
		}
	}

#pragma endregion deformation_data


	/*int AbstractAlgData::InsertToCircleBuffer(const GGTSAlg_Input& input)
	{
		if (input.byChannelID != by_channel_id_)
		{
			return 1;
		}
		else
		{
			for (int i = 0; i < input.i16HorLen; i++)
			{
				if (cb_ptr_->PushBack(input.i64Start + i, input.pDepth.get()[i]) == false)
					return 2;
			}
		}
		return 0;
	}*/


}

