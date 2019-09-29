#ifndef THREADS_H
#define THREADS_H

#include <assert.h>
#include <mutex>

namespace MyAlg
{
	template <class Host>
	class SingleThreaded
	{
	public:
		/// \struct Lock
		/// Dummy Lock class
		struct Lock
		{
			Lock() {}
			explicit Lock(const SingleThreaded&) {}
			explicit Lock(const SingleThreaded*) {}
		};

		typedef Host VolatileType;
	};


	////////////////////////////////////////////////////////////////////////////////
	///  \class ObjectLevelLockable
	///
	///  \ingroup ThreadingGroup
	///  Implementation of the ThreadingModel policy used by various classes
	///  Implements a object-level locking scheme
	////////////////////////////////////////////////////////////////////////////////
	template <class Host>
	class ObjectLevelLockable
	{
		mutable std::mutex mtx_;

	public:
		ObjectLevelLockable() : mtx_() {}
		ObjectLevelLockable(const ObjectLevelLockable&) : mtx_() {}
		~ObjectLevelLockable() {}

		class Lock;
		friend class Lock;

		///  \struct Lock
		///  Lock class to lock on object level
		class Lock
		{
		public:

			/// Lock object
			explicit Lock(const ObjectLevelLockable& host) : host_(host)
			{
				host_.mtx_.lock();
			}

			/// Lock object
			explicit Lock(const ObjectLevelLockable* host) : host_(*host)
			{
				host_.mtx_.lock();
			}

			/// Unlock object
			~Lock()
			{
				host_.mtx_.unlock();
			}

		private:
			/// private by design of the object level threading
			Lock();
			Lock(const Lock&);
			Lock& operator=(const Lock&);
			const ObjectLevelLockable& host_;
		};

		typedef volatile Host VolatileType;
	};


	////////////////////////////////////////////////////////////////////////////////
		///  \class ClassLevelLockable
		///
		///  \ingroup ThreadingGroup
		///  Implementation of the ThreadingModel policy used by various classes
		///  Implements a class-level locking scheme
		////////////////////////////////////////////////////////////////////////////////
	template <class Host>
	class ClassLevelLockable
	{
		struct Initializer
		{
			bool init_;
			std::mutex mtx_;

			Initializer() : init_(false), mtx_()
			{
				init_ = true;
			}

			~Initializer()
			{
				assert(init_);
			}
		};

		static Initializer initializer_;

	public:

		class Lock;
		friend class Lock;

		///  \struct Lock
		///  Lock class to lock on class level
		class Lock
		{
		public:

			/// Lock class
			Lock()
			{
				assert(initializer_.init_);
				initializer_.mtx_.lock();
			}

			/// Lock class
			explicit Lock(const ClassLevelLockable&)
			{
				assert(initializer_.init_);
				initializer_.mtx_.lock();
			}

			/// Lock class
			explicit Lock(const ClassLevelLockable*)
			{
				assert(initializer_.init_);
				initializer_.mtx_.lock();
			}

			/// Unlock class
			~Lock()
			{
				assert(initializer_.init_);
				initializer_.mtx_.unlock();
			}

		private:
			Lock(const Lock&);
			Lock& operator=(const Lock&);
		};

		typedef volatile Host VolatileType;

	};
}



#endif