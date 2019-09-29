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
		//ԭʼ���ݱ���λ��
		std::vector<shared_ptr<channel_level_data> > channel_data;
		//�ָ����ݱ���λ��
		//��һ���±��ʾͨ���ţ��ڶ����±��ʾ��ͨ���ĵڼ�������
		std::vector< std::vector<shared_ptr<deformation_data> > > deform_data;
		//ȱ�ݿ�λ��
		std::list<shared_ptr<defect_box> > defect_box;
	};

}

#endif