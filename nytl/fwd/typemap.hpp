// Copyright (c) 2017-2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#ifndef NYTL_INCLUDE_FWD_TYPEMAP
#define NYTL_INCLUDE_FWD_TYPEMAP

#include <any>

namespace nytl {
	template<typename I, typename B = std::any, typename... CArgs> class Typemap;
}

#endif //header guard
