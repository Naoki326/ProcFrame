#include "ProcSolver.h"


namespace MyAlg
{

	bool ProcSolver::FromScript(string script)
		//, string cond_spt)
	{
		if (state_ != proc_state::initialized)
			return false;
		ReadParam(script);
		dataArr_.resize(lps_.size() + 1);
		for (int i = 0; i<dataArr_.size(); i++)
		{
			dataArr_[i].reset(new AlgData());
		}
		if (ProcsCreate(lps_, dataInd_, ProcArr_) != true)
			return false;
		if (ProcsInputParam(lps_, ProcArr_) != true)
			return false;
		state_ = proc_state::script_read;
		return true;
	}

	bool ProcSolver::ExecuteProc()
	{
		//等待Input达到一定数据
		if (state_ != proc_state::input_setted)
			return false;
		state_ = proc_state::excecuting;
		//执行层
		for (int i = 1; i < lps_.size() + 1; i++)
		{
			dataArr_[i].reset(new AlgData());
		}
		if (ProcsExecute(dataArr_, ProcArr_, lps_2_) != true)
		{
			state_ = proc_state::initialized;
			return false;
		}
		//设置数据使用情况
		//SetObsolete
		
		state_ = proc_state::executed;
		return true;
	}

	bool ProcSolver::ReleseData(unsigned long long right_est_x, unsigned int overlap)
	{
		int c_len = dataArr_[0]->channel_data.size();

		for (int i = 0; i < c_len; i++)
		{
			shared_ptr<channel_level_data> c_data = dataArr_[0]->channel_data[i];
			int c_data_len = c_data->GetLen();
			for (int j = 0; j < c_data_len; j++)
			{
				if (c_data->GetX(0) <= right_est_x - overlap)
				{
					c_data->DiscardFront();
				}
			}
			c_data->SetBasePos(right_est_x - overlap);
		}
		return true;
	}

	shared_ptr<AlgData> ProcSolver::At(string procName) const
	{
		shared_ptr<AlgData> resval;
		auto iter = dataInd_.find(procName);
		if (iter == dataInd_.end())
		{
			return resval;
		}
		if (iter->second+1 > dataArr_.size())
		{
			return resval;
		}
		return dataArr_[iter->second+1];
	}

	vector<string> ProcSolver::Split(const string& s, vector<string>& res, char delim)
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

	//将参数表s按两种方式排列读出（方式1：id，方式2：level）
	void ProcSolver::ReadParam(string s)
	{
		int levellen = 0;

		vector<string>res;
		Split(s, res, '\n');
		for (auto it = res.begin(); it < res.end(); it++)
		{
			vector<string> procParam;
			Split(*it, procParam, ';');
			ProcParam par;
			for (auto it2 = procParam.begin(); it2 < procParam.end(); it2++)
			{
				vector<string> param;
				Split(*it2, param, '=');
				param[0].erase(remove(param[0].begin(), param[0].end(), '\t'), param[0].end());
				param[0].erase(remove(param[0].begin(), param[0].end(), ' '), param[0].end());
				if (param.size() > 1)
				{
					param[1].erase(remove(param[1].begin(), param[1].end(), '\t'), param[1].end());
					param[1].erase(remove(param[1].begin(), param[1].end(), ' '), param[1].end());
				}
				else
				{
					param.push_back("");
				}
				ParamExplain(param[0], param[1], par);
			}
			if (par.level > levellen)
			{
				levellen = par.level;
			}

			lps_.push_back(par);
		}
		for (int i = 1; i <= levellen; i++)
		{
			vector<ProcParam> tmp;
			lps_2_.push_back(tmp);
			[&]()
			{
				auto it = lps_.begin();
				while (true)
				{
					it = find_if(it, lps_.end(), [=](ProcParam p)->bool {return p.level == i; });
					if (it == lps_.end())
					{
						return;
					}
					else
					{
						lps_2_[i - 1].push_back(*it);
						it++;
					}
				}
			}();
		}
	}

	int ProcSolver::ParamExplain(string param_lvalue, string param_rvalue, ProcParam & par)
	{
		if (param_lvalue == "id")
		{
			par.id = atoi(param_rvalue.c_str());
		}
		if (param_lvalue == "level")
		{
			par.level = atoi(param_rvalue.c_str());
		}
		if (param_lvalue == "type")
		{
			par.type = param_rvalue;
		}
		if (param_lvalue == "name")
		{
			par.name = param_rvalue;
		}
		if (param_lvalue == "param")
		{
			par.param = param_rvalue;
		}
		if (param_lvalue == "input")
		{
			vector<string> midp;
			Split(param_rvalue, midp, ',');
			if (midp.size() == 2)
			{
				par.input1 = atoi(midp[0].c_str());
				par.input2 = atoi(midp[1].c_str());
			}
			else
			{
				par.input1 = atoi(param_rvalue.c_str());
				par.input2 = -1;
			}
		}
		if (param_lvalue == "output")
		{
			par.output = atoi(param_rvalue.c_str());
		}
		return -1;
	}

	bool ProcSolver::ProcsCreate(vector<ProcParam>& lps_, map<string, int>& dataInd_, vector<shared_ptr<AbstractProc> >& tmpProcArr_)
	{
		//创建+传参
		for (int i = 0; i < lps_.size(); i++)
		{
			//创建
			shared_ptr<AbstractProc> proc = AlgFactory::GetInstance()->CreateObject(lps_[i].type);
			dataInd_.insert(std::make_pair(lps_[i].name, lps_[i].id));
			if (proc == nullptr)
			{
				//std::cout << "创建Proc错误";
				return false;
			}
			else
			{
				tmpProcArr_.push_back(proc);
			}
		}
		return true;
	}

	bool ProcSolver::ProcsInputParam(vector<ProcParam>& lps_, vector<shared_ptr<AbstractProc> >& tmpProcArr_)
	{
		//创建+传参
		for (int i = 0; i < lps_.size(); i++)
		{
			auto proc = tmpProcArr_[i];
			//传入参数
			int param_len = lps_[i].param.length() * sizeof(char) + 1;
			shared_ptr<char> mid(new char[param_len], std::default_delete<char[]>());
			memcpy(mid.get(), lps_[i].param.c_str(), param_len);
			if (proc->SetParam(mid, param_len) == false)
				return false;

		}
		return true;
	}

	//执行函数，需要修改
	bool ProcSolver::ProcsExecute(vector<shared_ptr<AlgData> >& dataArr_,
		vector<shared_ptr<AbstractProc> >& ProcArr_,
		vector<vector<ProcParam> >& lps_2_)
	{
		//分层执行
		for (int i = 1; i <= lps_2_.size(); i++)
		{
			//这里可以并行执行
			for (int j = 0; j < lps_2_[i - 1].size(); j++)
			{
				ProcParam lp = lps_2_[i - 1][j];
				int id = lp.id;
				int input1 = lp.input1;
				int input2 = lp.input2;
				int output = lp.output;
				string name = lp.name;
				shared_ptr<AbstractProc> proc = ProcArr_[lp.id];
				
				if (proc->Execute(dataArr_[input1], dataArr_[output]) != true)
					return false;
			}
		}
		return true;
	}

	bool ProcSolver::SetInput(const shared_ptr<AlgData>& data)
	{
		this->dataArr_[0] = data;
		return true;
	}

	void ProcSolver::Step()
	{
		if(this->state_ != proc_state::exit)
			this->state_ = proc_state::input_setted;
	}


}