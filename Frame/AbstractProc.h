#ifndef BASEPROC_H
#define BASEPROC_H

#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>

#include "Factory.h"
#include "../DataStruct/AlgData.h"


namespace MyAlg
{
	using std::string;
	using std::shared_ptr;
	using std::accumulate;
	using std::sort;
	using std::find_if;
	using std::max_element;
	using std::min_element;
	using std::vector;

	//算法抽象类
	class AbstractProc
	{
	public:
		AbstractProc(){}
		//指针传递参数，类型不安全
		virtual bool SetParam(shared_ptr<char>param, int param_len) = 0;
		virtual bool Execute(const shared_ptr<AlgData>& src, shared_ptr<AlgData>& dst) = 0;

	protected:
		vector<string> Split(const string& s, vector<string>& res, char delim)
		{
			//cv::LINE_AA
			std::stringstream ss(s);
			string item;
			while (std::getline(ss, item, delim))
			{
				res.push_back(item);
			}
			return res;
		}
	};
	
	//将算法类用于工厂模式的定义
	typedef shared_ptr<AbstractProc>(*ProductCreator)();
	typedef MyFactory::Factory<AbstractProc, string, ProductCreator> AlgFactory;
	
	class ProductRegisterer
	{
	public:
		ProductRegisterer(const string& id, ProductCreator creator)
		{
			AlgFactory::GetInstance()->Register(id, creator);
		}
	};

	//工厂类注册
#define REGISTER_PROC_CREATOR(name)													\
	shared_ptr<AbstractProc> Create##name()											\
	{return shared_ptr<AbstractProc>(new name##Proc); }								\
	static ProductRegisterer g_creator_##name(#name, Create##name);						\





}
#endif