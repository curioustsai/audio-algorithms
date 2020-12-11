#include <gtest/gtest.h>

extern "C" {
#include "fftwrap.h"
}

TEST(widget, ok) {
	ASSERT_EQ(widget_ok(1, 1), 1);
}

// TEST(widget, not_ok) {
// 	ASSERT_EQ(widget_ok(1, 2), 0);
// }
