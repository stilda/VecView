#include "VecView.h"

// Support const T* and T* cases
template<typename Ptr>
struct ArrayAccessor
{
	typename std::add_reference<typename std::remove_pointer<Ptr>::type>::type operator[](int idx) const 
	{ 
		return ptr[idx]; 
	}
	ArrayAccessor(Ptr ptr) : ptr(ptr) {}
private:
	Ptr const ptr;
};

template< typename Storage >
class View
{
	const Storage st;
	const int dim;
public:
	View(Storage st, int dim) : st(st), dim(dim) {}
	View<Storage> & operator= (int a)
	{
		for (int i = 0; i < dim; i++)
		{
			//st[i] = 5;
		}
		return *this;
	}
};

template<typename Ptr>
View<ArrayAccessor<Ptr>> view(Ptr ptr, int  dim)
{
	return View<ArrayAccessor<Ptr>>(ArrayAccessor<Ptr>(ptr), dim);
}

int main()
{
	int arr[] = { 1, 2 };
	auto v = view(arr, 2);
	v = 4;

	const int carr[] = { 1, 2 };
	auto cv = view(carr, 2);
	cv = 3;

	vevi::tests();

	return 0;
}