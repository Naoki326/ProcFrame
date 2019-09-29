#ifndef STANDARDPROC_H
#define STANDARDPROC_H

#include "../Frame/AbstractProc.h"
//#include "AbstractProc.h"

namespace MyAlg
{
	//标准过程
	class StandardProc : public AbstractProc
	{
	private:

	public:
		StandardProc()
		{}
		bool SetParam(shared_ptr<char> param, int param_len)
		{
			shared_ptr<char> mid(new char[param_len], std::default_delete<char[]>());
			memcpy(mid.get(), param.get(), param_len);
			//thresh_ = atof(mid.get());
			return true;
		}

		bool Execute(const shared_ptr<AlgData>& src, shared_ptr<AlgData>& dst)
		{
			return true;
		}

	};
	REGISTER_PROC_CREATOR(Standard)

}

#endif