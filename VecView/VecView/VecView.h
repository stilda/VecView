//
// This library provides operations on multidimentional vectors:
//  - arithmetic operations like negation, addition, subtraction, dot prod, l2 norm, scalar-vector operations, etc
//  - statistical operations like min/max coordinate, mean, std.dev., etc
//  - casting types
//
// This library does not provide implementation of Vector type with storage. 
// Instead it operates on Vector Views that require storage interface for construction, and calls its operator [] to access elements of vector.
//
// Arguments of operations can be const views of vectors.
// There is a special assignable view results of computation can be assigned to.
//
// Because of storage absense, no intermediate results of operations can be created (during evaluation of expressions). 
// So expressions computed "by coordinates".
// There is only one loop through coordinates of assignable vector and whole expression is evaluated one coordinate by another. 
// For example expression v = v1 + 3*v2 + v3 equivalent to for(int i = 0; i < dim; ++i) { v[i] = v1[i] + 3*v2[i] + v3[i]; }
//
// Vector view can be with specified dimention and without it. 
// You can omit dimention specification everythere where it is possible to infer it from operations' mathematical semantic.
// For example when you add two vectors you are allowed to specify dimention for only one of them. The other will be supposed to have the same dimention. 
// Or you can write Dot( v1 + v2, v3 + v4 ) where only v4 has specified dimention.
//
// There are several types of Vector View:
//  - Const Vector View with specified dimention
//  - Const Vector View with no dimention specification
//  - Assignable Vector View with specified dimention
//  - Number View
// Number view is a wrapper class for scalars that sometimes acts as scalar and sometimes as any-dimentional vector.
// For example ConstVector1 + Number(3) adds 3 to each component of vector, ConstVector1 * Number(3) multiplies each component by 3 and
// Number(3) + Number(4) just adds to scalar numbers producing Number(7)
//
// To create Assignable Vector View use AVec(...), to create Const Vector View (with and without dimention specification) use Vec(...), 
// to use scalar numbers in expressions wrap them in Num(...).
//
// TODO:
// 1. Ensure inlining of everything to get a single loop over coordinates like shown in example above.
// 2. Add much more operaions.
//
// Use examples:
//
// int v1[] = {1,2,3}; 
// int v2[] = {3,4,5};
// int v[3];
// AVec(v,3) = Vec(v1) + Vec(v2); // v = {4,6,8}
// int dotprod = Dot(Vec(v1,3),Vec(v2,3)); // 26
// int dotprod2 = Dot(Vec(v1,3),Vec(v2)); // 26
// int dotprod3 = Dot(Vec(v1) + Vec(v1), Vec(v2,3)); // equivalent to Dot({2,4,6},{3,4,5}) = 52
// int dotprod4 = Dot(Vec(v1),Vec(v2)); // compile time error, no way to infer dimentions of vectors
//
// AVec(v,3) = Num(2) + Num(3)*Vec(v1); // v = {5,8,11}
// AVec(v,3) = Num(1) + Dot(Vec(v1,2),Vec(v1)); // v = {6,6,6}
//
// int i = Num(2); // i = 2;
//
// double v3[] = {1.0, 2.0, 3.0};
// float v4[3];
// AVec(v4,3) = Cast<float>(Vec(v3));
// 


#pragma once

#include <type_traits>

namespace vevi
{
	namespace details
	{
		// Standard storages of coordinates for which Views can be created.
		// User can define his own storages. It has to have members: ElementType, operator[], support move semantics.
		namespace storages
		{
			// Storage interface that is essentially pointer to an array. Support const T* and T* cases
			template<typename Ptr>
			struct ArrayPtr
			{
				using ElementType = typename std::remove_const<typename std::remove_pointer<Ptr>::type>::type;
				typename std::add_reference<typename std::remove_pointer<Ptr>::type>::type operator[](int idx) const
				{
					return ptr[idx];
				}
				ArrayPtr(const Ptr & ptr) : ptr(ptr) {}
			private:
				const Ptr ptr;
			};

			// Storage interface that is essentially pointer to strided array. Support const T* and T* cases
			template<typename Ptr>
			struct StridedArrayPtr
			{
				using ElementType = typename std::remove_const<typename std::remove_pointer<Ptr>::type>::type;
				typename std::add_reference<typename std::remove_pointer<Ptr>::type>::type operator[](int idx) const
				{
					return ptr[idx*stride];
				}
				StridedArrayPtr(const Ptr & ptr, int stride) : ptr(ptr), stride(stride) {}
			private:
				Ptr const ptr;
				const int stride;
			};

			// Storage interface that allocates and owns memory for coordinates.
			template<typename T>
			struct OwnedArray
			{
				using ElementType = T;
				T & operator[](int idx) const
				{
					return buf[idx];
				}
				OwnedArray(int dim) { buf = new T[dim]; }
				~OwnedArray() { if (buf) delete[] buf; }
				OwnedArray(OwnedArray<T> && oa)
				{
					buf = oa.buf;
					oa.buf = nullptr;
				}
			private:
				OwnedArray(const OwnedArray<T> & oa){}
				T * buf = nullptr;
			};
		}

		template<typename T>
		class NumberView
		{
			const T num;
		public:
			using type = T;
			NumberView(T num) : num(num) {}
			T Evaluate(int) const { return num; }
			operator T() const { return num; }
		};

		template<typename Storage>
		class VectorView
		{
			const int dim;
			const Storage storage;
		public:
			using type = typename Storage::ElementType;
			VectorView(Storage storage, int dim) : dim(dim), storage(std::move(storage)) {}
			type Evaluate(int i) const { return storage[i]; }
			int Dim() const { return dim; }
		};

		// Not Dimentional Vector is a vector with no dimention specified. Usage of it is controlled by other vectors' dimentions in expression.
		// User is responsible for checking that it has enough length to compute expression.
		template<typename Storage>
		class NoDimVectorView
		{
			const Storage storage;
		public:
			using type = typename Storage::ElementType;
			NoDimVectorView(Storage storage) : storage(std::move(storage)) {}
			type Evaluate(int i) const { return storage[i]; }
		};

		template<typename Storage>
		class AssignableVectorView
		{
			const int dim;
			const Storage storage;
		public:
			using type = typename Storage::ElementType;
			AssignableVectorView(Storage storage, int dim) : dim(dim), storage(std::move(storage)) {}
			type Evaluate(int i) const { return storage[i]; }
			int Dim() const { return dim; }
			type operator[](int i) const { return storage[i]; }

			template<typename Expr>
			AssignableVectorView<Storage> & operator=(const Expr & expr)
			{
				for (int i = 0; i < dim; ++i)
					storage[i] = expr.Evaluate(i);
				return *this;
			}
		};

		// Helper class to check if class has a member function "int Dim() const"
		template <typename T>
		class HasMemberDim
		{
			typedef char Yes;
			typedef Yes No[2];
			template <typename U, U> struct really_has;
			template <typename C> static Yes& Test(really_has<int (C::*)() const, &C::Dim>*);
			template <typename> static No& Test(...);
		public:
			static bool const value = sizeof(Test<T>(0)) == sizeof(Yes);
		};

		// If first argument has Dim then use it, otherwise use Dim of the second argument.
		// If there is no Dim of the second argument there will be compilation error 
		// meaning that vector operation can not be performed because dimentionality is not known
		template<typename Arg1, typename Arg2>
		struct Dimention
		{
			template<typename U = Arg1>
			static typename std::enable_if<HasMemberDim<U>::value, int>::type Dim(const Arg1 & a1, const Arg2 & a2)
			{
				return a1.Dim();
			}

			template<typename U = Arg1>
			static typename std::enable_if<!HasMemberDim<U>::value, int>::type Dim(const Arg1 & a1, const Arg2 & a2)
			{
				return a2.Dim();
			}
		};

		template<typename Arg1, typename Arg2>
		struct VectorAdd
		{
			using type = decltype(Arg1::type() + Arg2::type());
			static type run(int i, const Arg1 & v1, const Arg2 & v2)
			{
				return v1.Evaluate(i) + v2.Evaluate(i);
			}
		};

		template<typename Arg1, typename Arg2>
		struct VectorSub
		{
			using type = decltype(Arg1::type() - Arg2::type());
			static type run(int i, const Arg1 & v1, const Arg2 & v2)
			{
				return v1.Evaluate(i) - v2.Evaluate(i);
			}
		};

		template<typename Arg1, typename Arg2>
		struct DotProd
		{
			using type = decltype(Arg1::type() * Arg2::type());
			static type run(const Arg1 & d1, const Arg2 & d2)
			{
				type res = type(0);
				for (int i = 0; i < Dimention<Arg1, Arg2>::Dim(d1, d2); ++i)
					res += d1.Evaluate(i) * d2.Evaluate(i);
				return res;
			}
		};

		template<typename Arg1>
		struct VectorNeg
		{
			using type = decltype(-Arg1::type());
			static type run(int i, const Arg1 & v1)
			{
				return -v1.Evaluate(i);
			}
		};

		template<typename Arg1, typename TargetType>
		struct VectorCast
		{
			using type = TargetType;
			static type run(int i, const Arg1 & v1)
			{
				return (type)v1.Evaluate(i);
			}
		};

		template<template <typename, typename, typename...> class Op, typename Arg1, typename Arg2, typename ...Args>
		class BinOp
		{
			const Arg1 & v1;
			const Arg2 & v2;
		public:
			using type = typename Op<Arg1, Arg2, Args...>::type;
			BinOp(const Arg1 & v1, const Arg2 & v2) : v1(v1), v2(v2) {}
			type Evaluate(int i) const { return Op<Arg1, Arg2, Args...>::run(i, v1, v2); }

			template<typename U = Arg1, typename V = Arg2>
			typename std::enable_if<HasMemberDim<U>::value || HasMemberDim<V>::value, int>::type
				Dim() const { return details::Dimention<U, V>::Dim(v1, v2); }
		};

		template<template <typename, typename...> class Op, typename Arg1, typename ... Args >
		class UnaOp
		{
			const Arg1 & v;
		public:
			using type = typename Op<Arg1, Args...>::type;
			UnaOp(const Arg1 & v) : v(v) {}
			type Evaluate(int i) const { return Op<Arg1, Args...>::run(i, v); }

			template<typename U = Arg1>
			typename std::enable_if<HasMemberDim<U>::value, int>::type
				Dim() const { return v.Dim(); }
		};

	}

	// Assignable Vector
	template<typename T>
	inline details::AssignableVectorView<details::storages::ArrayPtr<T*>> AVec(T * ptr, int dim)
	{
		return{ { ptr }, dim };
	}
	template<typename T>
	inline details::AssignableVectorView<details::storages::StridedArrayPtr<T*>> AVec(T * ptr, int dim, int stride)
	{
		return{ { ptr, stride }, dim };
	}
	template<typename T>
	inline details::AssignableVectorView<details::storages::OwnedArray<T>> AVec(int dim)
	{
		return{ { dim }, dim };
	}

	// Const Vector
	template<typename T>
	inline details::VectorView<details::storages::ArrayPtr<const T*>> Vec(const T * ptr, int dim)
	{
		return{ { ptr }, dim };
	}
	template<typename T>
	inline details::VectorView<details::storages::StridedArrayPtr<const T*>> Vec(const T * ptr, int dim, int stride)
	{
		return{ { ptr, stride }, dim };
	}

	// Const Vector without dimention
	template<typename T>
	inline details::NoDimVectorView<details::storages::ArrayPtr<const T*>> Vec(const T * ptr)
	{
		return{ { ptr } };
	}
	//template<typename T>
	//inline details::NoDimVectorView<details::storages::StridedArrayPtr<const T*>> Vec(const T * ptr, int stride) 
	//{ 
	//	return {{ptr,stride}};
	//}

	// Wrap for numbers
	template<typename T>
	inline details::NumberView<T> Num(T num) { return details::NumberView<T>(num); }

	// Functions on Numbers
	template<typename T>
	inline details::NumberView<T> operator+(const details::NumberView<T> & v1, const details::NumberView<T> & v2)
	{
		return T(v1) + T(v2);
	}

	// Bin operations
	template<typename Arg1, typename Arg2>
	inline details::BinOp<details::VectorAdd, Arg1, Arg2> operator+(const Arg1 & v1, const Arg2 & v2)
	{
		return details::BinOp<details::VectorAdd, Arg1, Arg2>(v1, v2);
	}

	template<typename Arg1, typename Arg2>
	inline details::BinOp<details::VectorSub, Arg1, Arg2> operator-(const Arg1 & v1, const Arg2 & v2)
	{
		return details::BinOp<details::VectorSub, Arg1, Arg2>(v1, v2);
	}

	template<typename Arg1, typename Arg2>
	inline details::NumberView<typename details::DotProd<Arg1, Arg2>::type> Dot(const Arg1 & v1, const Arg2 & v2)
	{
		return details::DotProd<Arg1, Arg2>::run(v1, v2);
	}

	// Unary operations
	template<typename Arg1>
	inline details::UnaOp<details::VectorNeg, Arg1> operator-(const Arg1 & v)
	{
		return details::UnaOp<details::VectorNeg, Arg1>(v);
	}

	template<typename TargetType, typename Arg1>
	inline details::UnaOp<details::VectorCast, Arg1, TargetType> Cast(const Arg1 & v)
	{
		return details::UnaOp<details::VectorCast, Arg1, TargetType>(v);
	}

	bool tests();
}
