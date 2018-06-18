#ifndef STD_REBIND_HPP
#define STD_REBIND_HPP

// std::rebind_t implementation
// Author: Charles Salvia, csalvia@bloomberg.net

#include <type_traits>
#include <tuple>

namespace std { namespace experimental { 

namespace detail {

	// Implementation of void_t for pre-C++17 compilers
	//
	template <class... T> 
	struct void_t_impl 
	{ 
		typedef void type;
	};

	template <class... T>
	using void_t = typename detail::void_t_impl<T...>::type;


	template <class C, class ParameterTuple, class Enable = void>
	struct instantiate_from_tuple
	{ };

	template <
		template <class...> class C,
		class... Params1,
		class... Params2
	>
	struct instantiate_from_tuple<
		C<Params1...>,
		std::tuple<Params2...>,
		void_t<C<Params2...>>
	>
	{
		using type = C<Params2...>;
	};

	template <class C, class ParameterTuple>
	using instantiate_from_tuple_t = typename instantiate_from_tuple<
		C, 
		ParameterTuple
	>::type;

	// Helper meta-function to determine the number of default template arguments in a class
	// template.  Here we attempt to instantiate class template C with a decreasing number
	// of template parameters, until a substitution failure occurs, at which point we
	// can infer the number of default template arguments.
	//
	template <class C, class ParamTuple, std::size_t Count, class Void = void, class Enable = void>
	struct default_argument_count_impl
	{
		constexpr static std::size_t value = Count;
	};

	// Helper templates to pop the last type from a tuple type
	//
	template <class Tuple>
	struct pop_back_impl;

	template <class Tuple, class... Args>
	struct do_pop_back;

	template <class... Args1, class T, class... Args2>
	struct do_pop_back<std::tuple<Args1...>, T, Args2...>
	{
		using type = typename do_pop_back<std::tuple<Args1..., T>, Args2...>::type;
	};

	template <class... Args, class T>
	struct do_pop_back<std::tuple<Args...>, T>
	{
		using type = std::tuple<Args...>;
	};

	template <class... Args>
	struct pop_back_impl<std::tuple<Args...>>
	{
		using type = typename do_pop_back<std::tuple<>, Args...>::type;
	};

	// Attempt to pop the last type from a tuple type, or trigger a substitution failure
	//
	template <class Tuple, class Enable = void>
	struct pop_back_or_substitution_failure
	{ };
	
	template <class Tuple>
	struct pop_back_or_substitution_failure<
		Tuple,
		typename std::enable_if<
			std::tuple_size<Tuple>::value != 0
		>::type
	>
	{
		using type = typename pop_back_impl<Tuple>::type;
	};

	template <
		class... Params1,
		template <class...> class C,
		class... Params2,
		std::size_t Count
	>
	struct default_argument_count_impl<
		C<Params1...>,
		std::tuple<Params2...>,
		Count,
		void_t<
			instantiate_from_tuple_t<
				C<Params1...>, 
				typename pop_back_or_substitution_failure<std::tuple<Params2...>>::type
			>
		>
	>
	{
		constexpr static std::size_t value = default_argument_count_impl<
			instantiate_from_tuple_t<
				C<Params1...>, 
				typename pop_back_or_substitution_failure<std::tuple<Params2...>>::type
			>,
			typename pop_back_or_substitution_failure<std::tuple<Params2...>>::type,
			Count + 1
		>::value;
	};

	template <class C>
	struct default_argument_count;

	template <
		template <class...> class C,
		class... Params
	>
	struct default_argument_count<C<Params...>>
	{
		constexpr static std::size_t value = default_argument_count_impl<
			C<Params...>,
			std::tuple<Params...>,
			0
		>::value;
	};

	template <class C> 
	struct template_parameters
	{
		using parameters = std::tuple<>;
	};

	template <
		template <class...> class C, 
		class... Params1
	>
	struct template_parameters<C<Params1...>>
	{
		template <class... Params2, std::size_t... I>
		static auto rebind(
			std::tuple<Params2...>, 
			std::index_sequence<I...>
		)
			->
				C<
					Params2...,
					std::tuple_element_t<
						I + sizeof... (Params2),
						std::tuple<Params1...>
					>...
				>
		;

		using parameters = std::tuple<Params1...>;
	};

	namespace static_test {

		template <class A = void, class B = void, class C = void, class D = void, class E = void>
		struct C1 { };

		template <class A, class B = void, class C = void, class D = void, class E = void>
		struct C2 { };

		template <class A, class B, class C = void, class D = void, class E = void>
		struct C3 { };

		template <class A, class B, class C, class D = void, class E = void>
		struct C4 { };

		template <class A, class B, class C, class D, class E = void>
		struct C5 { };

		template <class A, class B, class C, class D, class E>
		struct C6 { };

		static_assert(
			detail::default_argument_count<C1<int,int,int,int,int>>::value == 5,
			"Internal library error"
		);

		static_assert(
			detail::default_argument_count<C2<int,int,int,int,int>>::value == 4,
			"Internal library error"
		);

		static_assert(
			detail::default_argument_count<C3<int,int,int,int,int>>::value == 3,
			"Internal library error"
		);

		static_assert(
			detail::default_argument_count<C4<int,int,int,int,int>>::value == 2,
			"Internal library error"
		);

		static_assert(
			detail::default_argument_count<C5<int,int,int,int,int>>::value == 1,
			"Internal library error"
		);

		static_assert(
			detail::default_argument_count<C6<int,int,int,int,int>>::value == 0,
			"Internal library error"
		);

	} // end namespace static_test

} // end namespace detail


// Retrieve the Nth template parameter type from class template C
//
template <class C, std::size_t N>
struct template_parameter
{
	using type = typename std::tuple_element<
		N, 
		typename detail::template_parameters<C>::parameters
	>::type;
};

template <class C, std::size_t N>
using template_param_t = typename template_parameter<
	C,
	N
>::type;

// Retrieve the number of template parameters associated with class template C
//
template <class C>
struct template_parameter_count
{
	constexpr static std::size_t value = std::tuple_size<
		typename detail::template_parameters<C>::parameters
	>::value;
};

// Determine if class C is a class template
//
template <class C>
struct is_template 
	: 
	std::integral_constant<
		bool,
		template_parameter_count<C>::value != 0
	>
{ };

namespace detail {

	template <class C, class ParameterTuple, class Enable = void>
	struct rebind_impl
	{
		static_assert(
			is_template<C>::value,
			"Class used with rebind_t is not a class template"
		);
	};

	template <template <class...> class C, class... Params1, class... Params2>
	struct rebind_impl<
		C<Params1...>,
		std::tuple<Params2...>,
		typename std::enable_if<
			(sizeof...(Params1) != 1) && (sizeof...(Params2) != 0)
		>::type
	>
	{
		static_assert(
			sizeof...(Params1) >= sizeof... (Params2),
			"rebind_t: too many rebound template parameters"
		);

		constexpr static std::size_t old_params = sizeof... (Params1);
		constexpr static std::size_t new_params = sizeof...(Params2);
		constexpr static std::size_t num_defaults = detail::default_argument_count<
			C<Params1...>
		>::value;

		constexpr static std::size_t num_required = old_params - num_defaults;
		constexpr static std::size_t diff = old_params - new_params;
		constexpr static std::size_t seq_size = diff - (
			(new_params > num_required) ? (num_defaults - (new_params - num_required)) : num_defaults
		);

		using type = decltype(
			detail::template_parameters<C<Params1...>>::rebind(
				std::tuple<Params2...>{},
				std::make_index_sequence<seq_size>{}
			)
		);
	};

	template <template <class...> class C, class... Params1, class... Params2>
	struct rebind_impl<
		C<Params1...>,
		std::tuple<Params2...>,
		typename std::enable_if<(
			(sizeof...(Params1) == 1 && sizeof...(Params2) == 1)
		)>::type
	>
	{
		using type = C<Params2...>;
	};

	// ----- specialization for 0 rebound template parameters
	//
	template <template <class...> class C, class... Params1, class... Params2>
	struct rebind_impl<
		C<Params1...>,
		std::tuple<Params2...>,
		typename std::enable_if<(sizeof...(Params2) == 0)>::type
	>
	{
		using type = C<Params1...>;
	};

} // end namespace detail


// Rebind class template C to the specified parameter list
//
template <class C, class... Params>
struct rebind;

template <template <class...> class C, class... Params1, class... Params2>
struct rebind<
	C<Params1...>,
	Params2...
>
	: detail::rebind_impl<C<Params1...>, std::tuple<Params2...>>
{ };

template <class C, class... Params>
using rebind_t = typename rebind<C, Params...>::type;

namespace detail { namespace static_test {

	template <class T>
	struct alloc
	{ };

	static_assert(
		std::is_same<
			rebind_t<alloc<int>, float>,
			alloc<float>
		>::value,
		"Internal library error"
	);

	template <class T = int>
	struct c0
	{ };

	static_assert(
		std::is_same<
			rebind_t<c0<int>, float>,
			c0<float>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c0<int>>,
			c0<int>
		>::value,
		"Internal library error"
	);

	template <class A, class B, class C>
	struct a0
	{ };

	static_assert(
		std::is_same<
			rebind_t<a0<int, float, double>, char, short, long>,
			a0<char, short, long>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<a0<int, float, double>, char, short>,
			a0<char, short, double>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<a0<int, float, double>, char>,
			a0<char, float, double>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<a0<int, float, double>>,
			a0<int, float, double>
		>::value,
		"Internal library error"
	);

	template <class T, class Alloc = alloc<T>>
	struct c1
	{ };

	static_assert(
		std::is_same<
			rebind_t<c1<int>, float>,
			c1<float, alloc<float>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c1<int>, float, alloc<double>>,
			c1<float, alloc<double>>
		>::value,
		"Internal library error"
	);

	template <class A, class B, class C, class D = alloc<A>, class E = alloc<B>, class F = alloc<C>>
	struct c2
	{ };

	struct X {};
	struct Y {};
	struct Z {};

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>>,
			c2<long, float, long double, alloc<long>, alloc<float>, alloc<long double>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>, X>,
			c2<X, float, long double, alloc<X>, alloc<float>, alloc<long double>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>, X, Y>,
			c2<X, Y, long double, alloc<X>, alloc<Y>, alloc<long double>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>, X, Y, Z>,
			c2<X, Y, Z, alloc<X>, alloc<Y>, alloc<Z>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>, X, Y, Z, float>,
			c2<X, Y, Z, float, alloc<Y>, alloc<Z>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>, X, Y, Z, float, alloc<float>>,
			c2<X, Y, Z, float, alloc<float>, alloc<Z>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>, X, Y, Z, float, alloc<float>, alloc<double>>,
			c2<X, Y, Z, float, alloc<float>, alloc<double>>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<c2<long, float, long double>>,
			c2<long, float, long double>
		>::value,
		"Internal library error"
	);


	template <class... T>
	struct B 
	{ };

	static_assert(
		std::is_same<
			rebind_t<B<int, long, float>, double>,
			B<double>
		>::value,
		"Internal library error"
	);

	static_assert(
		std::is_same<
			rebind_t<B<int, long, float>, double, char>,
			B<double, char>
		>::value,
		"Internal library error"
	);

} } // end namespace detail::static_test

} } // end namespace std::experimental

#endif


