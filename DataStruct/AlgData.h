#ifndef ALGDATA_H
#define ALGDATA_H

#include <vector>
#include <list>
#include "AbstractData.h"

namespace MyAlg
{

	class AlgData
	{
	public:
		//原始数据保存位置
		std::vector<shared_ptr<channel_level_data> > channel_data;
		//分割数据保存位置
		//第一个下标表示通道号，第二个下标表示该通道的第几个数据
		std::vector< std::vector<shared_ptr<deformation_data> > > deform_data;
		//缺陷框位置
		std::list<shared_ptr<defect_box> > defect_box;
	};

}

#endif