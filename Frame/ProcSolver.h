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

	//�㷨ִ�С����ݹ�����
	class ProcSolver : public ObjectLevelLockable<ProcSolver>
	{
	private:
		enum proc_state { initialized, script_read, input_setted, excecuting, executed, exit };

	public:
		ProcSolver()
		{
			state_ = proc_state::initialized;
		}

		//�����㷨�Ľű��ļ�
		bool FromScript(string script);//, string cond_spt);
		//�����㷨��������
		bool SetInput(const shared_ptr<AlgData>& data);
		void Step();
		//ִ���㷨
		bool ExecuteProc();
		//unsigned long long LeftEstX();
		bool ReleseData(unsigned long long right_est_x, unsigned int ed);
		//�����㷨���
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

		//��������s�����ַ�ʽ���ж�������ʽ1��id����ʽ2��level��
		void ReadParam(string s);

		int ParamExplain(string param_lvalue, string param_rvalue, ProcParam & par);

		bool ProcsCreate(vector<ProcParam>& lps_, map<string, int>& dataInd_, vector<shared_ptr<AbstractProc> >& tmpProcArr_);

		bool ProcsInputParam(vector<ProcParam>& lps_, vector<shared_ptr<AbstractProc> >& tmpProcArr_);

		bool ProcsExecute(vector<shared_ptr<AlgData> >& dataArr_,
			vector<shared_ptr<AbstractProc> >& ProcArr_,
			vector<vector<ProcParam> >& lps_2_);


	private:
		vector<ProcParam> lps_;							//����˳�����Ĳ���
		vector<vector<ProcParam> > lps_2_;				//��level˳�����Ŷ���Ĳ���

		vector<shared_ptr<AbstractProc> > ProcArr_;		//Procʵ��
		map<string, int> dataInd_;						//������Proc����֮�������

		vector<shared_ptr<AlgData> > dataArr_;					//�������

		proc_state state_;								//ʼ�ռ�¼ǰһ״̬

	};
}








#endif