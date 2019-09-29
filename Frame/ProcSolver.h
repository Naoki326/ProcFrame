#ifndef PROCSOLVER_H
#define PROCSOLVER_H

#include <sstream>

#include "AbstractProc.h"
#include "../Tools/Threads.h"
#include "../DataStruct/AlgData.h"

namespace MyAlg
{
	using std::vector;
	using std::shared_ptr;
	using std::map;

	struct ProcParam
	{
		int id;
		int level;
		string type;
		string name;
		string param;
		int input1;
		int input2;
		int output;
		shared_ptr<AbstractData> data;

		ProcParam() : id(-1), level(-1), type(""), name(""), param(""), input1(-1), input2(-1), data(nullptr)
		{}
	};

	//算法执行、数据管理类
	class ProcSolver : public ObjectLevelLockable<ProcSolver>
	{
	private:
		enum proc_state { initialized, script_read, input_setted, excecuting, executed, exit };

	public:
		ProcSolver()
		{
			state_ = proc_state::initialized;
		}

		//读入算法的脚本文件
		bool FromScript(string script);//, string cond_spt);
		//设置算法输入数据
		bool SetInput(const shared_ptr<AlgData>& data);
		void Step();
		//执行算法
		bool ExecuteProc();
		//unsigned long long LeftEstX();
		bool ReleseData(unsigned long long right_est_x, unsigned int ed);
		//读出算法结果
		shared_ptr<AlgData> At(string procName) const;

		void TryExit()
		{
			Lock lock(this);
			state_ = proc_state::exit;
		}

		bool ReadyToExit()
		{
			Lock lock(this);
			return state_ == proc_state::exit;
		}

	private:
		vector<string> Split(const string& s, vector<string>& res, char delim);

		//将参数表s按两种方式排列读出（方式1：id，方式2：level）
		void ReadParam(string s);

		int ParamExplain(string param_lvalue, string param_rvalue, ProcParam & par);

		bool ProcsCreate(vector<ProcParam>& lps_, map<string, int>& dataInd_, vector<shared_ptr<AbstractProc> >& tmpProcArr_);

		bool ProcsInputParam(vector<ProcParam>& lps_, vector<shared_ptr<AbstractProc> >& tmpProcArr_);

		bool ProcsExecute(vector<shared_ptr<AlgData> >& dataArr_,
			vector<shared_ptr<AbstractProc> >& ProcArr_,
			vector<vector<ProcParam> >& lps_2_);


	private:
		vector<ProcParam> lps_;							//按行顺序读入的参数
		vector<vector<ProcParam> > lps_2_;				//按level顺序重排读入的参数

		vector<shared_ptr<AbstractProc> > ProcArr_;		//Proc实体
		map<string, int> dataInd_;						//数据与Proc名字之间的链接

		vector<shared_ptr<AlgData> > dataArr_;					//存放数据

		proc_state state_;								//始终记录前一状态

	};
}








#endif