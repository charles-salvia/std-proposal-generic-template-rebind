#include "rebind.hpp"
#include <vector>
#include <string>

using vec_type1 = std::vector<int>;
using vec_type2 = std::experimental::rebind_t<vec_type1, std::string>;

static_assert(
	std::is_same<
		vec_type2,
		std::vector<std::string, std::allocator<std::string>>
	>::value,
	"Fail"
);

int main()
{
	// Test is compile-time only
}
