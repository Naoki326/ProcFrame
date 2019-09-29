#ifndef NK_FACTORY_H
#define NK_FACTORY_H
	
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace MyFactory
{

	template
	<
		class AbstractProduct,
		typename IdentifierType = std::string,
		typename ProductCreator = AbstractProduct* (*)
	>
	class Factory
	{
	private:
		typedef std::map<IdentifierType, ProductCreator> IdToProductMap;
		IdToProductMap associations_;

		Factory() : associations_()
		{
		}
		Factory& operator = (const Factory& t);
		Factory(const Factory &);

	public:
		static Factory* GetInstance()
		{
			static Factory instan_;
			return &instan_;
		}

		~Factory()
		{
			associations_.erase(associations_.begin(), associations_.end());
		}

		bool Register(const IdentifierType& id, ProductCreator creator)
		{
			return associations_.insert(
				typename IdToProductMap::value_type(id, creator)).second != 0;
		}

		bool Unregister(const IdentifierType& id)
		{
			return associations_.erase(id) != 0;
		}

		std::vector<IdentifierType> RegisteredIds()
		{
			std::vector<IdentifierType> ids;
			for (typename IdToProductMap::iterator it = associations_.begin();
				it != associations_.end(); ++it)
			{
				ids.push_back(it->first);
			}
			return ids;
		}

		std::shared_ptr<AbstractProduct> CreateObject(const IdentifierType& id)
		{
			typename IdToProductMap::iterator i = associations_.find(id);
			if (i != associations_.end())
				return (i->second)();
			return nullptr;
		}
	};

	
}


#endif